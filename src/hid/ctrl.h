#pragma once
#ifndef DSY_KNOB_H
#define DSY_KNOB_H /**< & */
#include <stdint.h>

#ifdef __cplusplus
namespace daisy
{
/**
    @brief Hardware Interface for control inputs \n 
    Primarily designed for ADC input controls such as \n 
    potentiometers, and control voltage. \n 
    @author Stephen Hensley
    @date November 2019
    @ingroup controls
*/
class AnalogControl
{
  public:
    /** Constructor */
    AnalogControl() {}
    /** destructor */
    ~AnalogControl() {}

    /** 
    Initializes the control
    \param *adcptr is a pointer to the raw adc read value -- This can be acquired with dsy_adc_get_rawptr(), or dsy_adc_get_mux_rawptr()
    \param sr is the samplerate in Hz that the Process function will be called at.
    \param flip determines whether the input is flipped (i.e. 1.f - input) or not before being processed.1
    \param invert determines whether the input is inverted (i.e. -1.f * input) or note before being processed.
    \param slew_seconds is the slew time in seconds that it takes for the control to change to a new value.
    */
    void Init(uint16_t *adcptr,
              float     sr,
              bool      flip         = false,
              bool      invert       = false,
              float     slew_seconds = 0.002f);

    /** 
    This Initializes the AnalogControl for a -5V to 5V inverted input
    All of the Init details are the same otherwise
    \param *adcptr Pointer to analog digital converter
    \param sr Audio engine sample rate
    */
    void InitBipolarCv(uint16_t *adcptr, float sr);

    /** 
    Filters, and transforms a raw ADC read into a normalized range.
    this should be called at the rate of specified by samplerate at Init time.   
    Default Initializations will return 0.0 -> 1.0
    Bi-polar CV inputs will return -1.0 -> 1.0
    */
    float Process();

    /** Returns the current stored value, without reprocessing */
    inline float Value() const { return val_; }

    /** Sets the Scale/Offset for the processor 
     ** A 0-3v3 input would use scale =1 offset = 0 
     ** A typical -5V to +5V CV input circuit would be inverted, and have a scale of 2, and offset of 0.5
     ** */
    inline void SetScaleOffset(float scale, float offset)
    {
        scale_  = scale;
        offset_ = offset;
    }

    /** Adjusts the Filter Coefficient to use.*/
    inline void SetFilterSlewTime(float slew_seconds)
    {
        coeff_ = 1.0f / (slew_seconds * samplerate_ * 0.5f);
    }

  private:
    uint16_t *raw_;
    float     coeff_, samplerate_, val_;
    float     scale_, offset_;
    bool      flip_;
    bool      invert_;
};
} // namespace daisy
#endif
#endif
