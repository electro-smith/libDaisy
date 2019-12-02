// # port
// Applies portamento to an input signal. At each new step value, the input is low-pass filtered to 
// move towards that value at a rate determined by ihtim. ihtim is the “half-time” of the 
// function (in seconds), during which the curve will traverse half the distance towards the new value, 
// then half as much again, etc., theoretically never reaching its asymptote.
//
// This code has been ported from Soundpipe to DaisySP by Paul Batchelor. 
// The Soundpipe module was extracted from the Csound opcode "portk".
// Original Author(s): Robbin Whittle, John ffitch
// Year: 1995, 1998
// Location: Opcodes/biquad.c

#pragma once
#ifndef PORT_H
#define PORT_H
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
// - sample_rate: sample rate of audio engine
// - htime: half-time of the function, in seconds.

// ~~~~
		void init(float sample_rate, float htime);
// ~~~~

// ### process
// Applies portamento to input signal and returns processed signal. 

// ~~~~
		float process(float in);
// ~~~~

// ## Setters

// ### sethtime_
// Sets htime

// ~~~~
		inline void set_htime(float htime) { htime_ = htime; }
// ~~~~

// ## Getters

// ### gethtime_
// returns current value of htime

// ~~~~
		inline float get_htime() { return htime_; }
// ~~~~
	private:
		float htime_;
    	float c1_, c2_, yt1_, prvhtim_;
    	float sample_rate_, onedsr_;
	};
} // namespace daisysp
#endif
#endif