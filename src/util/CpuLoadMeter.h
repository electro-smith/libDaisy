#pragma once

#include "sys/system.h"
#include <cmath>

namespace daisy
{
/** @brief CPU load metering
 *  @author jelliesen
 *  @addtogroup utility
 * 
 *  To measure the CPU load of your audio processing, create a CpuLoadMeter
 *  and initialize it with your block size and sample rate.
 *  Then at the beginning of the audio callback, call `OnBlockStart()`, 
 *  and at the end of the audio callback, call `OnBlockEnd()`.
 *  You can then read out the minimum, maximum and average CPU load.
 */
class CpuLoadMeter
{
  public:
    CpuLoadMeter(){};

    /** Initializes the CpuLoadMeter for a particular sample rate and block size.
     *  @param sampleRateInHz           The sample rate in Hz
     *  @param blockSizeInSamples       The block size in samples
     *  @param smoothingFilterCutoffHz  The cutoff frequency of the smoothing filter that's used to
     *                                  create the average CPU load reading.
     */
    void Init(float sampleRateInHz,
              int   blockSizeInSamples,
              float smoothingFilterCutoffHz = 1.0f)
    {
        const auto secPerBlock = float(blockSizeInSamples) / sampleRateInHz;
        const auto ticksPerS   = float(System::GetTickFreq());
        ticksPerBlockInv_      = 1.0f / (ticksPerS * secPerBlock);

        // update filter coefficient for smoothing filter (1pole lowpass)
        const auto blockRateInHz = sampleRateInHz / float(blockSizeInSamples);
        const auto cutoffNormalized
            = smoothingFilterCutoffHz * 2.0f * 3.141592653f / blockRateInHz;
        // according to
        // https://en.wikipedia.org/wiki/Low-pass_filter#Simple_infinite_impulse_response_filter
        smoothingConstant_ = cutoffNormalized / (cutoffNormalized + 1.0f);

        Reset();
    }

    /** Call this at the beginning of your audio callback */
    void OnBlockStart() { currentBlockStartTicks_ = System::GetTick(); }

    /** Call this at the end of your audio callback */
    void OnBlockEnd()
    {
        const auto end         = System::GetTick();
        const auto ticksPassed = end - currentBlockStartTicks_;
        const auto currentBlockLoad
            = float(ticksPassed) * ticksPerBlockInv_; // usPassed / usPerBlock

        if(firstCycle_)
        {
            max_ = min_ = avg_ = currentBlockLoad;
            firstCycle_        = false;
        }
        else
        {
            if(currentBlockLoad > max_)
                max_ = currentBlockLoad;
            if(currentBlockLoad < min_)
                min_ = currentBlockLoad;

            avg_ = smoothingConstant_ * currentBlockLoad
                   + (1.0f - smoothingConstant_) * avg_;
        }
    }

    /** Returns the smoothed average CPU load in the range 0..1 */
    float GetAvgCpuLoad() const { return avg_; }
    /** Returns the minimun CPU load observed since the last call to Reset(). */
    float GetMinCpuLoad() const { return min_; }
    /** Returns the maximum CPU load observed since the last call to Reset(). */
    float GetMaxCpuLoad() const { return max_; }

    /** Resets the minimun, maximum and average load readings. */
    void Reset()
    {
        firstCycle_ = true;
        avg_ = max_ = min_ = NAN;
    }

  private:
    bool     firstCycle_;
    float    ticksPerBlockInv_;
    uint32_t currentBlockStartTicks_;
    float    min_;
    float    max_;
    float    avg_;
    float    smoothingConstant_;

    CpuLoadMeter(const CpuLoadMeter&) = delete;
    CpuLoadMeter& operator=(const CpuLoadMeter&) = delete;
};
} // namespace daisy