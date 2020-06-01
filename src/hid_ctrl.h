/** Hardware Interface for control inputs
*/
//
/** Primarily designed for ADC input controls such as
potentiometers, and control voltage.
**Author:** Stephen Hensley
**Date Added:** November 2019
*/
//
#pragma once
#ifndef DSY_KNOB_H
#define DSY_KNOB_H
#include <stdint.h>

#ifdef __cplusplus
namespace daisy
{
class AnalogControl
{
  public:
    AnalogControl() {}
    ~AnalogControl() {}

/** Initializes the control
*adcptr is a pointer to the raw adc read value --
This can acquired with dsy_adc_get_rawptr(), or dsy_adc_get_mux_rawptr()
*/
    //
/** sr is the samplerate in Hz that the Process function will be called at.
*/
    //
/** slew_seconds is the slew time in seconds that it takes for the control to change to a new value.
*/
    //
/** flip determines whether the input is flipped (i.e. 1.f - input) or not before being processed.
*/
	//
/** invert determines whether the input is inverted (i.e. -1.f * input) or note before being processed.
*/
    //
    void Init(uint16_t *adcptr,
              float     sr,
              bool      flip         = false,
              bool      invert       = false,
              float     slew_seconds = 0.002f);

/** This Initializes the AnalogControl for a -5V to 5V inverted input
*/
    //
/** All of the Init details are the same otherwise
*/
    void InitBipolarCv(uint16_t *adcptr, float sr);

/** filters, and transforms a raw ADC read into a normalized range.
*/
    //
/** this should be called at the rate of specified by samplerate at Init time.
*/
    //
/** Default Initializations will return 0.0 -> 1.0
*/
    //
/** Bi-polar CV inputs will return -1.0 -> 1.0
*/
    float Process();

/** Returns the current stored value, without reprocessing
*/
    inline float Value() const { return val_; }

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
