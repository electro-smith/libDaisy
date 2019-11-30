// # metro 
// Creates a clock signal at specific frequency

#pragma once
#ifndef METRO_H
#define METRO_H
#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{
	class metro
	{
	public:
		metro(){}
		~metro() {}

// ### init
// Initializes metro module
// Arguments:
// - freq: frequency at which new clock signals will be generated
// 	Input Range: 
// - sample_rate: sample rate of audio engine
// 	Input range: 

// ~~~~
		void init(float freq, float sample_rate);
// ~~~~

// ### process
// checks current state of metro object and updates state if necesary.

// ~~~~
		uint8_t process();
// ~~~~

// ## Setters
// ### set_freq
// Sets frequency at which metro module will run at.

// ~~~~
		inline void set_freq(float freq) 
// ~~~~
		{
			_freq = freq;
			_phs_inc = (2.0f * M_PI * _freq) / _sample_rate;
		}

// ## Getters
// ### get_freq
// Returns current value for frequency.

// ~~~~
		inline float get_freq() { return _freq; }
// ~~~~

	private:
		float _freq;
		float _phs, _sample_rate, _phs_inc;

	};
} // namespace daisysp
#endif
#endif
