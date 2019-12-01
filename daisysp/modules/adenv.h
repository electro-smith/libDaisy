
// # adenv

// Author: shensley
//
// 		Trigger-able envelope with adjustable min/max, and independent per-segment time control.
// 
// TODO:
// - Add Cycling
// - Implement Curve (its only linear for now).
// - Maybe make this an AD_sr that has AD/AR/A_sr modes.

#pragma once
#ifndef ADENV_H
#define ADENV_H
#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{

// ## Envelope Segments
// 
// Distinct stages that the phase of the envelope can be located in. 
// - IDLE = located at phase location 0, and not currently running
// - ATTACK = First segment of envelope where phase moves from MIN value to MAX value
// - DECAY = Second segment of envelope where phase moves from MAX to MIN value
// - LAST = The final segment of the envelope (currently decay)
// ~~~~
	enum {
			ADENV_SEG_IDLE,
			ADENV_SEG_ATTACK,
			ADENV_SEG_DECAY,
			ADENV_SEG_LAST,
	};
// ~~~~
	
	class adenv
	{
		public:
		adenv() {}
		~adenv() {} 

// ### init
//
// Initializes the ad envelope
//
// float sample_rate - sample rate of the audio engine being run.
//
// Defaults
// - current segment = idle
// - curve = linear
// - phase = 0
// - min = 0
// - max = 1
//
// ~~~~
		void init(float sample_rate);
// ~~~~

// ### process
// processes the current sample of the envelope. Returns the current envelope value. This should be called once per sample period. 
// ~~~~
		float process();
// ~~~~

// ### trigger
// Starts or retriggers the envelope. 
// ~~~~
		inline void trigger() {_trigger = 1; }
// ~~~~

// ## Setters

// ### set_segment_time
// Sets the length of time(secondsVERIFYTHIS) for a specific segment.
// ~~~~
		inline void set_time(uint8_t seg, float time)
// ~~~~
		{
			_segment_time[seg] = time; 
		}

// ### set_curve_scalar
// Sets the amount of curve applied. 
// Input range: -1 to 1. 
// - At -1, curve = full logarithmic
// - At 1, curve = full exponential
// - At 0, curve = linear

// ~~~~
		inline void set_curve_scalar(float scalar) { _curve_scalar = scalar; }
// ~~~~

// ### set_min
//
// Sets the minimum value of the envelope output
// Input range: -FLT_MAX, to FLT_MAX
// ~~~~		
		inline void set_min(float min) {_min = min; }
// ~~~~

// ### set_max
//
// Sets the maximum value of the envelope output
// Input range: -FLT_MAX, to FLT_MAX
// ~~~~
		inline void set_max(float max) {_max = max; }
// ~~~~

// ## Getters

// ### current_segment
//
// Returns the segment of the envelope that the phase is currently located in. 
// ~~~~		
		inline void current_segment();
// ~~~~
		private:
		void calculate_multiplier(float start, float end, uint32_t length_in_samples);
		uint8_t _current_segment;
		float _segment_time[ADENV_SEG_LAST];
		float _sr, _phase_inc, _min, _max, _multiplier, _output, _curve_scalar;
		uint32_t _phase;
		uint8_t _trigger;
	};

} // namespace daisysp
#endif
#endif
