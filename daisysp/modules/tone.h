// # tone
//
// A first-order recursive low-pass filter with variable frequency response.
// 
#pragma once
#ifndef DSY_TONE_H
#define DSY_TONE_H

#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{
	class tone
	{
	public:
		tone() {}
		~tone() {}
// ### init
//
// Initializes the tone module.
//
// sample_rate - The sample rate of the audio engine being run. 
// 
// ~~~~
		void init(float sample_rate);
// ~~~~

// ### process
//
// Processes one sample through the filter and returns one sample.
//
// in - input signal 
// 
// ~~~~
		float process(float &in);
// ~~~~

// ## Setters
// 
// ### set_freq
// 
// Sets the cutoff frequency or half-way point of the filter.
// 
// Arguments
//
// - freq - frequency value in Hz. Range: Any positive value.
//
// ~~~~
		inline void set_freq(float &freq) 
// ~~~~
		{
			freq_ = freq;
    		calculate_coefficients();
		}

// ## Getters
// 
// ### get_freq
// 
// Returns the current value for the cutoff frequency or half-way point of the filter.
//
// ~~~~
		inline float get_freq() { return freq_; }
// ~~~~

	private:
		void calculate_coefficients();
		float out_, prevout_, in_, freq_, c1_, c2_, sample_rate_;
	};
} // namespace daisysp
#endif
#endif
