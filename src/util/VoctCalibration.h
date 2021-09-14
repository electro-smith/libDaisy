#pragma once

namespace daisy
{
/** @brief Helper class for calibrating an input to 1V/oct response 
 *  @author shensley
 * 
 *  This provides a scale and offset value for converting
 *  incoming CV into MIDI note numbers accurately for musical pitch tracking.
 * 
 *  To use, record both the 1V and 3V values using the specified functions
 *  once calibration is complete you can use the GetData function to retrieve
 *  the calibration values.
 * 
 *  This can also be used for 100mV/Semitone calibration as used by Buchla synthesizer 
 *  modules. To calibrate for this standard. You would send 1.2V, and 3.6V
 */
class VoctCalibration
{
  public:
    VoctCalibration() : scale_(0.f), offset_(0.f), cal_(false) {}

    ~VoctCalibration() {}

    bool Record(float val1V, float val3V)
    {
        float delta = val3V - val1V;
        if(delta < 0.6f && delta > 0.1f)
        {
            scale_  = 24.f / delta;
            offset_ = 12.f - scale_ * val1V;
            cal_    = true;
        }
        return cal_;
    }

    /** Get the scale and offset data from the calibration 
     *  \retval returns true if calibration has been performed.
    */
    bool GetData(float &scale, float &offset)
    {
        scale  = scale_;
        offset = offset_;
        return cal_;
    }

    /** Manually set the calibration data and mark internally as "calibrated" 
     *  This is used to reset the data after a power cycle without having to 
     *  redo the calibration procedure.
    */
    void SetData(float scale, float offset)
    {
        scale_  = scale;
        offset_ = offset;
        cal_    = true;
    }

    /** Process a value through the calibrated data to get a MIDI Note number */
    inline float ProcessInput(const float inval)
    {
        return offset_ + (scale_ * inval);
    }

  private:
    float scale_, offset_;
    bool  cal_;
};

} // namespace daisy