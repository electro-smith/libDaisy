#include "util/CpuLoadMeter.h"
#include <gtest/gtest.h>
#include <cmath>

using namespace daisy;

TEST(util_CpuLoadMeter, a_stateAfterInit)
{
    CpuLoadMeter meter;
    meter.Init(48000.0f, 32);
    EXPECT_TRUE(std::isnan(meter.GetMinCpuLoad()));
    EXPECT_TRUE(std::isnan(meter.GetMaxCpuLoad()));
    EXPECT_TRUE(std::isnan(meter.GetAvgCpuLoad()));
}

TEST(util_CpuLoadMeter, b_stateAfterReset)
{
    System::SetTickFreqForUnitTest(1000000u); // 1us tick duration
    CpuLoadMeter meter;
    meter.Init(48000.0f, 32);

    // make measurement
    meter.OnBlockStart();
    System::SetTickForUnitTest(System::GetTick() + 100);
    meter.OnBlockEnd();
    // check that this actually changes something
    EXPECT_FALSE(std::isnan(meter.GetMinCpuLoad()));
    EXPECT_FALSE(std::isnan(meter.GetMaxCpuLoad()));
    EXPECT_FALSE(std::isnan(meter.GetAvgCpuLoad()));

    // reset
    meter.Reset();

    EXPECT_TRUE(std::isnan(meter.GetMinCpuLoad()));
    EXPECT_TRUE(std::isnan(meter.GetMaxCpuLoad()));
    EXPECT_TRUE(std::isnan(meter.GetAvgCpuLoad()));
}

TEST(util_CpuLoadMeter, c_measureMinMax)
{
    System::SetTickFreqForUnitTest(1000000u); // 1us tick duration
    CpuLoadMeter meter;
    meter.Init(48000.0f, 48); // 1kHz block rate

    // measure block with 20% load
    meter.OnBlockStart();
    System::SetTickForUnitTest(System::GetTick() + 200);
    meter.OnBlockEnd();

    // check results
    EXPECT_FLOAT_EQ(meter.GetMinCpuLoad(), 0.2f);
    EXPECT_FLOAT_EQ(meter.GetMaxCpuLoad(), 0.2f);

    // measure block with 10% load
    meter.OnBlockStart();
    System::SetTickForUnitTest(System::GetTick() + 100);
    meter.OnBlockEnd();

    // check results
    EXPECT_FLOAT_EQ(meter.GetMinCpuLoad(), 0.1f);
    EXPECT_FLOAT_EQ(meter.GetMaxCpuLoad(), 0.2f);

    // measure block with 100% load
    meter.OnBlockStart();
    System::SetTickForUnitTest(System::GetTick() + 1000);
    meter.OnBlockEnd();

    // check results
    EXPECT_FLOAT_EQ(meter.GetMinCpuLoad(), 0.1f);
    EXPECT_FLOAT_EQ(meter.GetMaxCpuLoad(), 1.0f);
}

TEST(util_CpuLoadMeter, d_smoothedAverageLoad)
{
    System::SetTickFreqForUnitTest(1000000u); // 1us tick duration
    CpuLoadMeter meter;

    const auto processStepResponse = [&meter](int      numBlocks,
                                              uint32_t stepStartLoadUs,
                                              uint32_t stepEndLoadUs) {
        std::vector<float> results;
        for(int i = 0; i < numBlocks; i++)
        {
            meter.OnBlockStart();
            const bool isFirstValue = results.empty();
            System::SetTickForUnitTest(
                System::GetTick()
                + (isFirstValue ? stepStartLoadUs : stepEndLoadUs));
            meter.OnBlockEnd();
            results.push_back(meter.GetAvgCpuLoad());
        }
        return results;
    };
    const auto checkIsExponential = [](const std::vector<float>& values,
                                       float                     stepSizeInS,
                                       float                     cutoffFreqHz,
                                       float                     startValue,
                                       float                     targetValue) {
        const auto omega_0 = 2.0f * 3.141592653f * cutoffFreqHz;
        for(auto i = 0u; i < values.size(); i++)
        {
            const auto t             = float(i) * stepSizeInS;
            const auto expectedValue = startValue
                                       + (targetValue - startValue)
                                             * (1.0f - std::exp(-omega_0 * t));
            // more than 5% deviation?
            EXPECT_NEAR(values[i], expectedValue, 0.05f) << "at index " << i;
        }
    };

    const auto sampleRateInHz = 48000.0f;
    const auto blockSize      = 48;
    meter.Init(sampleRateInHz, blockSize);
    const auto numBlocks            = 1000; // 1s
    const auto blockRateInHz        = sampleRateInHz / float(blockSize);
    const auto stepSizeInS          = 1.0f / blockRateInHz;
    const auto expectedCutoffFreqHz = 1.0f;

    // check step response from 0% up to 100%
    const auto targetLoadTimeUs = 1.0f / blockRateInHz * 1000000.0f;
    const auto rampUp = processStepResponse(numBlocks, 0, targetLoadTimeUs);
    // is this an exponential step response with the right timing?
    checkIsExponential(rampUp, stepSizeInS, expectedCutoffFreqHz, 0, 1.0f);

    // check step response from 100% up to 0%
    meter.Reset();
    const auto startLoadTimeUs = 1.0f / blockRateInHz * 1000000.0f;
    const auto rampDown = processStepResponse(numBlocks, startLoadTimeUs, 0);
    // is this an exponential step response with the right timing?
    checkIsExponential(rampDown, stepSizeInS, expectedCutoffFreqHz, 1.0f, 0);
}

TEST(util_CpuLoadMeter, e_tolerateTickOverflow)
{
    System::SetTickFreqForUnitTest(1000000u); // 1us tick duration
    CpuLoadMeter meter;
    meter.Init(48000.0f, 48); // 1kHz block rate

    // measure block with 50% load where the tick counter overflows
    constexpr uint32_t counterMax = uint32_t(uint64_t(1ull << 32ul) - 1);
    constexpr uint32_t blockDurationInTicks
        = 1000; // 1kHz block rate and 1us tick
    constexpr uint32_t tickAtBlockStart = counterMax - blockDurationInTicks / 4;
    constexpr uint32_t tickAtBlockEnd
        = tickAtBlockStart + blockDurationInTicks / 2; // 50% load
    EXPECT_LT(tickAtBlockEnd, tickAtBlockStart); // this should have overflown the tick counter
    System::SetTickForUnitTest(tickAtBlockStart);
    meter.OnBlockStart();
    System::SetTickForUnitTest(tickAtBlockEnd);
    meter.OnBlockEnd();

    // check results - meter should have tolerated the overflow
    EXPECT_FLOAT_EQ(meter.GetMinCpuLoad(), 0.5f);
    EXPECT_FLOAT_EQ(meter.GetMaxCpuLoad(), 0.5f);
}