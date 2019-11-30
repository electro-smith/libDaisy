
// # crossfade
// Performs a crossfade between two signals
// 
// Original author: Paul Batchelor
//
// Ported from Soundpipe by Andrew Ikenberry
// added curve option for constant power, etc.

// TODO:
// - implement constant power curve process
// - implement exponential curve process
// - implement logarithmic curve process

#pragma once
#ifndef CROSSFADE_H
#define CROSSFADE_H
#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{

	// ## Curve Options
	// Curve that the crossfade will follow when processing
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
		// 
		// Initializes crossfade module
		// Defaults
		// - current position = .5
		// - curve = linear
		// 
		// ~~~~
		inline void init() 
		// ~~~~
		{
			_pos = 0.5;
    		_curve = CROSSFADE_LIN;
		}
		// ### process
		// processes crossfade and returns single sample 
		// 
		// ~~~~
		float process(float *in1, float *in2);
		// ~~~~

		// ## Setters

		// ### set_pos
		// Sets position of crossfade between two input signals
		// Input range: 0 to 1
		// ~~~~
		inline void set_pos(float pos) { _pos = pos; }
		// ~~~~

		// ### set_curve
		// Sets current curve applied to crossfade 
		// Expected input: See 'Curve Options' at top of page
		// ~~~~
		inline void set_curve(uint8_t curve) { _curve = curve; }
		// ~~~~

		// ## Getters

		// ### get_pos
		// Returns current position
		// ~~~~
		inline float get_pos(float pos) { return _pos; }
		// ~~~~

		// ### get_curve
		// Returns current curve
		// ~~~~
		inline uint8_t get_curve(uint8_t curve) { return _curve; }
		// ~~~~

	// private variables
	private:
		float _pos;
		uint8_t _curve;
	};
} // namespace daisysp
#endif
#endif