// # adenv
// 
// Author: shensley
//
// Trigger-able envelope with adjustable min/max, and independent per-segment time control.
// 
// TODO:
// 
// - Add Cycling
// - Implement Curve (its only linear for now).
// - Maybe make this an ADsr_ that has AD/AR/Asr_ modes.

#pragma once
#ifndef ADENV_H
#define ADENV_H
#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{

// ### Envelope Segments
// 
// Distinct stages that the phase of the envelope can be located in. 
// 
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
// 
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
		inline void trigger() {trigger_ = 1; }
// ~~~~

// ## Mutators

// ### setsegment_time_
// Sets the length of time(secondsVERIFYTHIS) for a specific segment.
// ~~~~
		inline void set_time(uint8_t seg, float time)
// ~~~~
		{
			segment_time_[seg] = time; 
		}

// ### setcurve_scalar_
// Sets the amount of curve applied. 
// Input range: -1 to 1. 
// - At -1, curve = full logarithmic
// - At 1, curve = full exponential
// - At 0, curve = linear

// ~~~~
		inline void set_curve_scalar(float scalar) { curve_scalar_ = scalar; }
// ~~~~

// ### set_min
//
// Sets the minimum value of the envelope output
// Input range: -FLTmax_, to FLTmax_
// ~~~~		
		inline void set_min(float min) {min_ = min; }
// ~~~~

// ### set_max
//
// Sets the maximum value of the envelope output
// Input range: -FLTmax_, to FLTmax_
// ~~~~
		inline void set_max(float max) {max_ = max; }
// ~~~~

// ## Accessors

// ### current_segment
//
// Returns the segment of the envelope that the phase is currently located in. 
// ~~~~		
		inline uint8_t get_current_segment() { return current_segment_; }
// ~~~~
//
 
// ### is_running
// Returns true if the envelope is currently in any stage apart from idle.
// ~~~~
        inline bool is_running() const { return current_segment_ != ADENV_SEG_IDLE; }
        // ~~~~

      private:
        
        void calculate_multiplier(float start, float end, uint32_t length_in_samples);
		uint8_t current_segment_, prev_segment_;
		float segment_time_[ADENV_SEG_LAST];
		float sample_rate_, phase_inc_, min_, max_, multiplier_, output_, curve_scalar_;
        float    c1_, curve_x_, retrig_val_;
        uint32_t phase_;
		uint8_t trigger_;
	};

} // namespace daisysp
#endif
#endif
