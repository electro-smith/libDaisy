// # metro 
// 
// Creates a clock signal at a specific frequency.
// 

#pragma once
#ifndef DSY_METRO_H
#define DSY_METRO_H
#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{
	class metro
	{
	public:
		metro() {}
		~metro() {}

// ### init
// 
// Initializes metro module.
// 
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

// ### reset
// resets phase to 0
// ~~~~
		inline void reset() { phs_ = 0.0f; }
// ~~~~

// ## Setters
// ### set_freq
// Sets frequency at which metro module will run at.

// ~~~~
		void set_freq(float freq);
// ~~~~

// ## Getters
// ### get_freq
// Returns current value for frequency.

// ~~~~
		inline float get_freq() { return freq_; }
// ~~~~

	private:
		float freq_;
		float phs_, sample_rate_, phs_inc_;

	};
} // namespace daisysp
#endif
#endif
