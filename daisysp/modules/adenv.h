// Attack Decay Envelope
// By shensley
//
// Trigger-able envelope with adjustable min/max, and independent per-segment time control.
// 
// TODO:
// - Add Cycling
// - Implement Curve (its only linear for now).
// - Maybe make this an AD_sr that has AD/AR/A_sr modes.
//
#pragma once
#ifndef ADENV_H
#define ADENV_H
#include <stdint.h>
#ifdef __cplusplus
namespace daisysp
{
	typedef enum {
			ADENV_SEG_IDLE,
			ADENV_SEG_RISE,
			ADENV_SEG_FALL,
			ADENV_SEG_LAST,
	} adenv_segment;

	class adenv
	{
		public:
		adenv() {}
		~adenv() {} 

		void init(float sample_rate);
		float process();

		inline void trigger()
		{
			_trigger = 1;		
		}

		inline void set__segment_time(adenv_segment seg, float time)
		{
			_segment_time[seg] = time;
		}

		inline void set_curve_scalar(float scalar)
		{
			_curve_scalar = scalar;	
		}

		inline void set_min_max(float min, float max)
		{
			_min = min;
			_max = max;
		}
		
		inline void current_segment();

		private:
		void calculate_multiplier(float start, float end, uint32_t length_in_samples);
		adenv_segment _current_segment;
		float _segment_time[ADENV_SEG_LAST];
		float _sr, _phase_inc, _min, _max, _multiplier, _output, _curve_scalar;
		uint32_t _phase;
		uint8_t _trigger;
	};

} // namespace daisysp
#endif
#endif
