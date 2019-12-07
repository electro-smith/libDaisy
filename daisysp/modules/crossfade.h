// # crossfade
// 
// Performs a crossfade between two signals
// 
// Original author: Paul Batchelor
//
// Ported from Soundpipe by Andrew Ikenberry
// added curve option for constant power, etc.
// 
// TODO:
// 
// - implement exponential curve process
// - implement logarithmic curve process

#pragma once
#ifndef DSY_CROSSFADE_H
#define DSY_CROSSFADE_H
#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{

// ## Curve Options
// Curve applied to the crossfade
// 
// - LIN = linear
// - CPOW = constant power
// - LOG = logarithmic
// - EXP  exponential
// - LAST = end of enum (used for array indexing)
// ~~~~
	enum 
	{
		CROSSFADE_LIN,
		CROSSFADE_CPOW,
		CROSSFADE_LOG,
		CROSSFADE_EXP,
		CROSSFADE_LAST,
	};
// ~~~~

	class crossfade
	{
	public:
		crossfade() {}
		~crossfade() {}

// ### init
// Initializes crossfade module
// 
// Defaults
// 
// - current position = .5
// - curve = linear
// 
// ~~~~
		inline void init() 
// ~~~~
		{
			pos_ = 0.5;
    		curve_ = CROSSFADE_LIN;
		}
// ### process
// processes crossfade and returns single sample 
// 
// ~~~~
		float process(float &in1, float &in2);
// ~~~~

// ## Setters

// ### set_pos
// Sets position of crossfade between two input signals
// 
// Input range: 0 to 1
// ~~~~
		inline void set_pos(float pos) { pos_ = pos; }
// ~~~~

// ### set_curve
// 
// Sets current curve applied to crossfade 
// 
// Expected input: See [Curve Options](##curve-options)
// ~~~~
		inline void set_curve(uint8_t curve) { curve_ = curve; }
// ~~~~

// ## Getters

// ### get_pos
// Returns current position
// ~~~~
		inline float get_pos(float pos) { return pos_; }
// ~~~~

// ### get_curve
// Returns current curve
// ~~~~
		inline uint8_t get_curve(uint8_t curve) { return curve_; }
// ~~~~

	private:
		float pos_;
		uint8_t curve_;
	};
} // namespace daisysp
#endif
#endif
