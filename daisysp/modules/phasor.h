// # phasor
// Generates a normalized signal moving from 0-1 at the specified frequency.
//
// TODO:
//
// I'd like to make the following things easily configurable:
//
// - Selecting which channels should be initialized/included in the sequence conversion.
// - Setup a similar start function for an external mux, but that seems outside the scope of this file.

#pragma once
#ifndef DSY_PHASOR_H
#define DSY_PHASOR_H
#ifdef __cplusplus

namespace daisysp
{
	class phasor
	{
	public:
		phasor() {}
		~phasor() {}

// ### init
// 
// Initializes the phasor module
// 
// sample rate, and freq are in Hz
// 
// initial phase is in radians
// 
// Additional init functions have defaults when arg is not specified:
//
// - phs = 0.0f
// - freq = 1.0f

// ~~~~
		inline void init(float sample_rate, float freq, float initial_phase) 
// ~~~~
		{
			sample_rate_ = sample_rate;
			phs_ = initial_phase;
			set_freq(freq);
		}

// ~~~~
		inline void init(float sample_rate, float freq) 
// ~~~~
		{
            init(sample_rate, freq, 0.0f);
		}

// ~~~~
		inline void init(float sample_rate) 
// ~~~~
		{ 
            init(sample_rate, 1.0f, 0.0f);
		}


// ### process
// processes phasor and returns current value
//
// ~~~~
		float process();
// ~~~~

// ## Setters

// ### set_freq
// Sets frequency of the phasor in Hz
// ~~~~
		void set_freq(float freq);
// ~~~~

// ## Getters

// ### get_freq
// Returns current frequency value in Hz
// ~~~~
		inline float get_freq() { return freq_; }
// ~~~~

	private:
		float freq_;
		float sample_rate_, inc_, phs_;

	};
} // namespace daisysp
#endif
#endif
