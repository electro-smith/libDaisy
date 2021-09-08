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
 */
class VoctCalibration
{
  public:
    VoctCalibration()
    : c1_(0.f),
      c3_(0.f),
      scale_(0.f),
      offset_(0.f),
      c1_read_(false),
      cal_(false)
    {
    }

    ~VoctCalibration() {}

    enum class Status
    {
        OK,
        ERR_INVALID_RANGE,
        ERR_CAL_OUT_OF_ORDER,
        ERR_CAL_NOT_COMPLETE,
    };

    /** Record the input value corresponding to 1 Volt */
    Status Record1V(float val)
    {
        c1_      = val;
        c1_read_ = true;
        return Status::OK;
    }

    /** Record the input value corresponding to 3 Volts,
     *  and calculate the calibration data.
     */
    Status Record3V(float val)
    {
        /** Ensure cal is being done in order */
        if(!c1_read_)
            return Status::ERR_CAL_OUT_OF_ORDER;

        c3_         = val;
        float delta = c3_ - c1_;
        if(delta < 0.6f && delta > 0.1f)
        {
            scale_  = 24.f / delta;
            offset_ = 12.f - scale_ * c1_;
            cal_    = true;
        }
        else
        {
            return Status::ERR_INVALID_RANGE;
        }
        return Status::OK;
    }

    /** Get the scale and offset data from the calibration */
    Status GetData(float *scale, float *offset)
    {
        if(cal_)
        {
            *scale  = scale_;
            *offset = offset_;
            return Status::OK;
        }
        return Status::ERR_CAL_NOT_COMPLETE;
    }


    /** Process a value through the calibrated data to get a MIDI Note number */
    inline float ProcessInput(const float inval)
    {
        return offset_ + (scale_ * inval);
    }

  private:
    float scale_, offset_;
    float c1_, c3_;
    bool  c1_read_, cal_;
};


} // namespace daisy