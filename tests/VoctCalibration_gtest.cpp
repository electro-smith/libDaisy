#include <gtest/gtest.h>
#include "util/VoctCalibration.h"

using namespace daisy;

TEST(util_VoctCalibration, a_stateAfterConstruction)
{
    VoctCalibration cal;
    float           scale, offset;
    bool            isCalibrated;

    /** Get Data without calibrating first. */
    isCalibrated = cal.GetData(scale, offset);

    /** Check that it reports uncalibrated */
    EXPECT_FALSE(isCalibrated);
    EXPECT_FLOAT_EQ(scale, 0.f);
    EXPECT_FLOAT_EQ(offset, 0.f);
}

TEST(util_VoctCalibration, b_recordValidRange)
{
    VoctCalibration cal;
    float           v1           = 0.2f; // Ideal CV input for 1V
    float           v3           = 0.6f; // Ideal CV input for 3V
    bool            isCalibrated = false;

    // Record with ideal values
    isCalibrated = cal.Record(v1, v3);

    // Get the new data
    float scale, offset;
    cal.GetData(scale, offset);

    // Verify values
    EXPECT_TRUE(isCalibrated);
    EXPECT_FLOAT_EQ(scale, 60.f);
		// Post-float-math, this comes out to ~9.5e-7
		// so float_eq is not going to work.
    EXPECT_NEAR(offset, 0.f, 1.0e-5f);
}

TEST(util_VoctCalibration, c_postCalibrationProcessing)
{
    VoctCalibration cal;
    // Calibrate with ideal values
    cal.Record(0.2f, 0.6f);

    // Loop through ideal octave inputs
    float octInput[]     = {0.f, 0.2f, 0.4f, 0.6f};
    float octMidiNotes[] = {0.f, 12.f, 24.f, 36.f};
    float testValues[4];
    for(int i = 0; i < 4; i++)
    {
        testValues[i] = cal.ProcessInput(octInput[i]);
    }

    // Check that the inputs map to the proper outputs.
		// Post-float-math, "0" comes out to ~9.5e-7
		// so float_eq is not going to work.
    EXPECT_NEAR(testValues[0], octMidiNotes[0], 1.0e-5f);
    EXPECT_FLOAT_EQ(testValues[1], octMidiNotes[1]);
    EXPECT_FLOAT_EQ(testValues[2], octMidiNotes[2]);
    EXPECT_FLOAT_EQ(testValues[3], octMidiNotes[3]);
}

TEST(util_VoctCalibration, d_manualValueSetting) 
{

    VoctCalibration cal;
    float           scale, offset;
    bool            isCalibrated;

		// Setting offset to non-default to test it changes
		cal.SetData(60.f, -1.f);

    /** Get Data without calibrating first. */
    isCalibrated = cal.GetData(scale, offset);

    /** Check that it reports uncalibrated */
    EXPECT_TRUE(isCalibrated);
    EXPECT_FLOAT_EQ(scale, 60.f);
    EXPECT_FLOAT_EQ(offset, -1.f);
}