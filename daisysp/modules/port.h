// # port
// Applies portamento to an input signal
//
// This code has been ported from Soundpipe to DaisySP by (paul batchelor?). 
// The Soundpipe module was extracted from the Csound opcode "portk".
// Original Author(s): Robbin Whittle, John ffitch
// Year: 1995, 1998
// Location: Opcodes/biquad.c

#pragma once
#ifndef PORT_H
#define PORT_H
#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{
	class port
	{
	public:
		port() {}
		~port() {}

// ### init
// Initializes port module
// Arguments:
// - sr: sample rate of audio engine
// - htime: time of portamento

// ~~~~
		void init(int sr, float htime);
// ~~~~

// ### process
// Applies portamento to input signal and returns processed signal. 

// ~~~~
		float process(float in);
// ~~~~

// ## Setters

// ### set_htime
// Sets htime

// ~~~~
		inline void set_htime(float htime) { _htime = htime; }
// ~~~~

// ## Getters

// ### get_htime
// returns current value of htime

// ~~~~
		inline float get_htime() { return _htime; }
// ~~~~
	private:
		float _htime;
    	float _c1, _c2, _yt1, _prvhtim;
    	float _sr, _onedsr;
	};
} // namespace daisysp
#endif
#endif