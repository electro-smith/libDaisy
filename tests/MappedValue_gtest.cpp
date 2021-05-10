#include <gtest/gtest.h>
#include "util/MappedValue.h"

using namespace daisy;

TEST(util_MappedFloatValue, a_default)
{
    MappedFloatValue val(0.0f, 1.0f, 0.5f);

    // value == default after construction
    EXPECT_EQ(val, 0.5f);
}

TEST(util_MappedFloatValue, b_set)
{
    MappedFloatValue val(0.0f, 1.0f, 0.5f);

    // setting via Set()
    val.Set(0.2f);
    EXPECT_EQ(val.Get(), 0.2f);
    val.Set(1.2f);
    EXPECT_EQ(val.Get(), 1.0f); // clamped to max
    val.Set(-0.5f);
    EXPECT_EQ(val.Get(), 0.0f); // clamped to min

    // setting via operator=(), getting via operator float()
    val = 0.2f;
    EXPECT_EQ(val, 0.2f);
    val = 1.2f;
    EXPECT_EQ(val, 1.0f); // clamped to max
    val = -0.5f;
    EXPECT_EQ(val, 0.0f); // clamped to min

    // resetting
    val.ResetToDefault();
    EXPECT_EQ(val.Get(), 0.5f);
}

TEST(util_MappedFloatValue, c_mapLin)
{
    MappedFloatValue val(-10.0f, 10.0f, 0.0f); // lin mapping by default

    // to mormalized value
    val = 0.0f;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.5f);
    val = 5.0f;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.75f);
    val = -10.0f;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.0f);

    // from normalized value
    val.SetFrom0to1(0.25f);
    EXPECT_FLOAT_EQ(val.Get(), -5.0f);
    val.SetFrom0to1(1.0f);
    EXPECT_FLOAT_EQ(val.Get(), 10.0f);
    val.SetFrom0to1(0.5f);
    EXPECT_FLOAT_EQ(val.Get(), 0.0f);
}

TEST(util_MappedFloatValue, d_mapLog)
{
    MappedFloatValue val(1.0f, 100.0f, 10.0f, MappedFloatValue::Mapping::log);

    // to mormalized value
    val = 1.0f;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.0f);
    val = 10.0f;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.5f);
    val = 100.0f;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 1.0f);

    // from normalized value
    val.SetFrom0to1(0.0f);
    EXPECT_FLOAT_EQ(val.Get(), 1.0f);
    val.SetFrom0to1(0.5f);
    EXPECT_FLOAT_EQ(val.Get(), 10.0f);
    val.SetFrom0to1(1.0f);
    EXPECT_FLOAT_EQ(val.Get(), 100.0f);
}

TEST(util_MappedFloatValue, e_mapPow2)
{
    MappedFloatValue val(0.0f, 10.0f, 0.0f, MappedFloatValue::Mapping::pow2);

    // to mormalized value
    val = 0.0f;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.0f);
    val = 5.0f;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.7071067811865475244f);
    val = 10.0f;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 1.0f);

    // from normalized value
    val.SetFrom0to1(0.0f);
    EXPECT_FLOAT_EQ(val.Get(), 0.0f);
    val.SetFrom0to1(0.7071067811865475244f);
    EXPECT_FLOAT_EQ(val.Get(), 5.0f);
    val.SetFrom0to1(1.0f);
    EXPECT_FLOAT_EQ(val.Get(), 10.0f);
}

TEST(util_MappedFloatValue, f_appendToStr)
{
    {
        // default: 1 decimal, no forced sign
        MappedFloatValue val(
            -10.0f, 10.0f, 0.0f, MappedFloatValue::Mapping::lin, "unit");
        FixedCapStr<20> str = "test";
        val                 = 5.5f;
        val.AppentToString(str);
        EXPECT_STREQ(str, "test5.5unit");
    }
    {
        // display "-" for negative numbers
        MappedFloatValue val(
            -10.0f, 10.0f, 0.0f, MappedFloatValue::Mapping::lin, "unit");
        FixedCapStr<20> str = "test";
        val                 = -5.5f;
        val.AppentToString(str);
        EXPECT_STREQ(str, "test-5.5unit");
    }
    {
        // more decimals
        MappedFloatValue val(
            -10.0f, 10.0f, 0.0f, MappedFloatValue::Mapping::lin, "unit", 2);
        FixedCapStr<20> str = "test";
        val                 = 5.5f;
        val.AppentToString(str);
        EXPECT_STREQ(str, "test5.50unit");
    }
    {
        // force sign for positive numbers
        MappedFloatValue val(-10.0f,
                             10.0f,
                             0.0f,
                             MappedFloatValue::Mapping::lin,
                             "unit",
                             1,
                             true);
        FixedCapStr<20>  str = "test";
        val                  = 5.5f;
        val.AppentToString(str);
        EXPECT_STREQ(str, "test+5.5unit");
    }
}

TEST(util_MappedFloatValue, g_stepUpDown)
{
    // step up and down, check that the normalised value changed
    // accordingly. Stepping changes the normalized value so
    // we don't have to repead this test for various mappings

    const auto       min                        = 1.0f;
    const auto       max                        = 100.0f;
    const auto       expectedNormStepSizeCoarse = 0.05f;
    const auto       expectedNormStepSizeFine   = 0.01f;
    MappedFloatValue val(min, max, 0.0f, MappedFloatValue::Mapping::log);

    // step up once
    val.SetFrom0to1(0.5f);
    val.Step(1, true);
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.5f + expectedNormStepSizeCoarse);

    val.SetFrom0to1(0.5f);
    val.Step(1, false);
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.5f + expectedNormStepSizeFine);

    // step up multiple times
    val.SetFrom0to1(0.5f);
    val.Step(5, true);
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.5f + 5 * expectedNormStepSizeCoarse);

    val.SetFrom0to1(0.5f);
    val.Step(5, false);
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.5f + 5 * expectedNormStepSizeFine);

    // step up and limit to max
    val.SetFrom0to1(0.5f);
    val.Step(100, true);
    EXPECT_EQ(val.GetAs0to1(), 1.0f);

    val.SetFrom0to1(0.5f);
    val.Step(100, false);
    EXPECT_EQ(val.GetAs0to1(), 1.0f);

    // step down once
    val.SetFrom0to1(0.5f);
    val.Step(-1, true);
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.5f - expectedNormStepSizeCoarse);

    val.SetFrom0to1(0.5f);
    val.Step(-1, false);
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.5f - expectedNormStepSizeFine);

    // step down multiple times
    val.SetFrom0to1(0.5f);
    val.Step(-5, true);
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.5f - 5 * expectedNormStepSizeCoarse);

    val.SetFrom0to1(0.5f);
    val.Step(-5, false);
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.5f - 5 * expectedNormStepSizeFine);

    // step down and limit to min
    val.SetFrom0to1(0.5f);
    val.Step(-100, true);
    EXPECT_EQ(val.GetAs0to1(), 0.0f);

    val.SetFrom0to1(0.5f);
    val.Step(-100, false);
    EXPECT_EQ(val.GetAs0to1(), 0.0f);
}

// ==========================================================================

// ==========================================================================

TEST(util_MappedIntValue, a_default)
{
    MappedIntValue val(0, 10, 5, 2, 1);

    // value == default after construction
    EXPECT_EQ(val, 5);
}

TEST(util_MappedIntValue, b_set)
{
    MappedIntValue val(-10, 10, 5, 2, 1);

    // setting via Set()
    val.Set(2);
    EXPECT_EQ(val.Get(), 2);
    val.Set(11);
    EXPECT_EQ(val.Get(), 10); // clamped to max
    val.Set(-30);
    EXPECT_EQ(val.Get(), -10); // clamped to min

    // setting via operator=(), getting via operator int()
    val = 2;
    EXPECT_EQ(val, 2);
    val = 11;
    EXPECT_EQ(val, 10); // clamped to max
    val = -30;
    EXPECT_EQ(val, -10); // clamped to min

    // resetting
    val.ResetToDefault();
    EXPECT_EQ(val.Get(), 5);
}

TEST(util_MappedIntValue, c_map)
{
    MappedIntValue val(-10, 10, 5, 2, 1);

    // to mormalized value
    val = 0;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.5f);
    val = 5;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.75f);
    val = -10;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.0f);

    // from normalized value
    val.SetFrom0to1(0.25f);
    EXPECT_FLOAT_EQ(val.Get(), -5);
    val.SetFrom0to1(1.0f);
    EXPECT_FLOAT_EQ(val.Get(), 10);
    val.SetFrom0to1(0.5f);
    EXPECT_FLOAT_EQ(val.Get(), 0);
}

TEST(util_MappedIntValue, d_appendToStr)
{
    {
        // default: print sign with negative numbers only
        MappedIntValue  val(-10, 10, 5, 2, 1, "unit");
        FixedCapStr<20> str = "test";
        val                 = 5;
        val.AppentToString(str);
        EXPECT_STREQ(str, "test5unit");
    }
    {
        // always print sign with negative numbers
        MappedIntValue  val(-10, 10, 5, 2, 1, "unit");
        FixedCapStr<20> str = "test";
        val                 = -10;
        val.AppentToString(str);
        EXPECT_STREQ(str, "test-10unit");
    }
    {
        // print sign with positive numbers when configured with forceSign = true
        MappedIntValue  val(-10, 10, 5, 2, 1, "unit", true);
        FixedCapStr<20> str = "test";
        val                 = 10;
        val.AppentToString(str);
        EXPECT_STREQ(str, "test+10unit");
    }
}

TEST(util_MappedIntValue, e_stepUpDown)
{
    // step up and down, check that the value changed
    // accordingly.

    const auto     min            = -10;
    const auto     max            = 10;
    const auto     stepSizeFine   = 1;
    const auto     stepSizeCoarse = 2;
    MappedIntValue val(min, max, 0, stepSizeFine, stepSizeCoarse, "");

    // step up once
    val = 0;
    val.Step(1, true);
    EXPECT_EQ(val, 0 + stepSizeCoarse);

    val = 0;
    val.Step(1, false);
    EXPECT_EQ(val, 0 + stepSizeFine);

    // step up multiple times
    val = -5;
    val.Step(5, true);
    EXPECT_EQ(val, -5 + 5 * stepSizeCoarse);

    val = -5;
    val.Step(5, false);
    EXPECT_EQ(val, -5 + 5 * stepSizeFine);

    // step up and limit to max
    val = 0;
    val.Step(100, true);
    EXPECT_EQ(val, max);

    val = 0;
    val.Step(100, false);
    EXPECT_EQ(val, max);

    // step down once
    val = 0;
    val.Step(-1, true);
    EXPECT_EQ(val, 0 - stepSizeCoarse);

    val = 0;
    val.Step(-1, false);
    EXPECT_EQ(val, 0 - stepSizeFine);

    // step up multiple times
    val = 5;
    val.Step(-5, true);
    EXPECT_EQ(val, 5 - 5 * stepSizeCoarse);

    val = 5;
    val.Step(-5, false);
    EXPECT_EQ(val, 5 - 5 * stepSizeFine);

    // step down and limit to min
    val = 0;
    val.Step(-100, true);
    EXPECT_EQ(val, min);

    val = 0;
    val.Step(-100, false);
    EXPECT_EQ(val, min);
}

// ==========================================================================

// ==========================================================================

TEST(util_MappedStringListValue, a_default)
{
    const char*           items[] = {"A", "B", "C"};
    MappedStringListValue val(items, 3, 1);

    // value == default after construction
    EXPECT_EQ(val, 1);
}

TEST(util_MappedStringListValue, b_set)
{
    const char*           items[] = {"A", "B", "C"};
    MappedStringListValue val(items, 3, 1);

    // setting via SetIndex()
    val.SetIndex(1);
    EXPECT_EQ(val.GetIndex(), 1);
    val.SetIndex(11);
    EXPECT_EQ(val.GetIndex(), 2); // clamped to max

    // setting via operator=(), getting via operator int()
    val = 1;
    EXPECT_EQ(val, 1);
    val = 11;
    EXPECT_EQ(val, 2); // clamped to max

    // resetting
    val.ResetToDefault();
    EXPECT_EQ(val.GetIndex(), 1);
}

TEST(util_MappedStringListValue, c_map)
{
    const char*           items[] = {"A", "B", "C"};
    MappedStringListValue val(items, 3, 1);

    // to mormalized value
    val = 0;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.0f);
    val = 1;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 0.5f);
    val = 2;
    EXPECT_FLOAT_EQ(val.GetAs0to1(), 1.0f);

    // from normalized value
    val.SetFrom0to1(0.32f);
    EXPECT_FLOAT_EQ(val.GetIndex(), 0); // rounded down
    val.SetFrom0to1(0.34f);
    EXPECT_FLOAT_EQ(val.GetIndex(), 1); // rounded up
    val.SetFrom0to1(0.65);
    EXPECT_FLOAT_EQ(val.GetIndex(), 1); // rounded down
    val.SetFrom0to1(0.67f);
    EXPECT_FLOAT_EQ(val.GetIndex(), 2); // rounded up
}

TEST(util_MappedStringListValue, d_appendToStr)
{
    const char*           items[] = {"A", "B", "C"};
    MappedStringListValue val(items, 3, 1);

    {
        FixedCapStr<20> str = "test";
        val                 = 1;
        val.AppentToString(str);
        EXPECT_STREQ(str, "testB");
    }
    {
        FixedCapStr<20> str = "test";
        val                 = 0;
        val.AppentToString(str);
        EXPECT_STREQ(str, "testA");
    }
}

TEST(util_MappedStringListValue, e_stepUpDown)
{
    // step up and down, check that the index changed
    // accordingly. Using the coarse stepsize should
    // jump to first/last index.

    const char*           items[] = {"A", "B", "C", "D", "E"};
    MappedStringListValue val(items, 5, 0);

    // step up once
    val = 0;
    val.Step(1, true);
    EXPECT_EQ(val, 4); // coarse => last index

    val = 0;
    val.Step(1, false);
    EXPECT_EQ(val, 1);

    // step up multiple times
    val = 0;
    val.Step(3, true);
    EXPECT_EQ(val, 4); // coarse => last index

    val = 0;
    val.Step(3, false);
    EXPECT_EQ(val, 3);

    // step up and limit to max
    val = 0;
    val.Step(100, false);
    EXPECT_EQ(val, 4);

    // step down once
    val = 4;
    val.Step(-1, true);
    EXPECT_EQ(val, 0); // coarse => first index

    val = 4;
    val.Step(-1, false);
    EXPECT_EQ(val, 3);

    // step up multiple times
    val = 4;
    val.Step(-3, true);
    EXPECT_EQ(val, 0); // coarse => first index

    val = 4;
    val.Step(-3, false);
    EXPECT_EQ(val, 1);

    // step down and limit to min
    val = 4;
    val.Step(-100, false);
    EXPECT_EQ(val, 0);
}