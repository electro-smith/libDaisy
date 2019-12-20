// # hid_ctrl
// ## Description
// Hardware Interface for control inputs
//
// Primarily designed for ADC input controls such as 
// potentiometers, and control voltage.
// ## Credit:
// **Author:** Stephen Hensley
// **Date Added:** November 2019
//
#pragma once
#ifndef DSY_KNOB_H
#define DSY_KNOB_H
#include <stdint.h>

#ifdef __cplusplus
namespace daisy
{
class hid_ctrl
{
  public:
	hid_ctrl() {}
	~hid_ctrl() {}

// ## General Functions
// ### init
// Initializes the control
// *adcptr is a pointer to the raw adc read value -- 
// This can acquired with dsy_adc_get_rawptr(), or dsy_adc_get_mux_rawptr()
// 
// sr is the samplerate in Hz that the process function will be called at.
// 
// slew_seconds is the slew time in seconds that it takes for the control to change to a new value.
// 
// slew_seconds defaults to 0.002 seconds if not specified.
// ~~~~
	void init(uint16_t *adcptr, float sr)
// ~~~~
	{
		val_		= 0.0f;
		raw_		= adcptr;
		samplerate_ = sr;
		coeff_		= 1.0f / (0.002f * samplerate_ * 0.5f);
		scale_		= 1.0f;
		offset_		= 0.0f;
		flip_		= false;
	}

// ~~~~
	void init(uint16_t *adcptr, float sr, float slew_seconds)
// ~~~~
	{
		val_		= 0.0f;
		raw_		= adcptr;
		samplerate_ = sr;
		coeff_		= 1.0f / (slew_seconds * samplerate_ * 0.5f);
		scale_		= 1.0f;
		offset_		= 0.0f;
		flip_		= false;
	}

// ### init_bipolar_cv
// This initializes the hid_ctrl for a -5V to 5V inverted input
// 
// All of the init details are the same otherwise 
//
// ~~~~
	void init_bipolar_cv(uint16_t *adcptr, float sr)
// ~~~~
	{
		val_		= 0.0f;
		raw_		= adcptr;
		samplerate_ = sr;
		coeff_		= 1.0f / (0.002f * samplerate_ * 0.5f);
		scale_  = 2.0f;
		offset_ = 0.5f;
		flip_   = true;
	}

// ### process
// filters, and transforms a raw ADC read into a normalized range.
// 
// this should be called at the rate of specified by samplerate at init time.
//
// Default Initializations will return 0.0 -> 1.0
// 
// Bi-polar CV inputs will return -1.0 -> 1.0
// ~~~~
	float process();
// ~~~~


  private:
	uint16_t *raw_;
	float	 coeff_, samplerate_, val_;
	float	 scale_, offset_;
	bool	  flip_;
};
} // namespace daisy
#endif
#endif
