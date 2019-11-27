#pragma once
#ifndef DSY_OSCILLATOR_H
#define DSY_OSCILLATOR_H
#include <stdint.h>
#ifdef __cplusplus
namespace daisysp
{
	class oscillator
	{
	  public:
		oscillator() {}
		~oscillator() {}

		enum
		{
			WAVE_SIN,	
			WAVE_TRI,
			WAVE_SAW,
			WAVE_RAMP,
			WAVE_SQUARE,
			WAVE_POLYBLEP_TRI,
			WAVE_POLYBLEP_SAW,
			WAVE_POLYBLEP_SQUARE,
			WAVE_LAST,
		};

		void init(float samplerate)
		{
			sr		  = samplerate;
			freq	  = 100.0f;
			amp		  = 0.5f;
			phase	 = 0.0f;
			phase_inc = calc_phase_inc(freq);
			waveform  = WAVE_SIN;
		}

		inline void set_freq(const float f) 
		{
			freq	  = f;
			phase_inc = calc_phase_inc(f);
		}

		inline void set_amp(const float a) { amp = a; }

		inline void set_waveform(const uint8_t wf) { waveform = wf < WAVE_LAST ? wf : WAVE_SIN; }

		float process();

	  private:

		inline float calc_phase_inc(float f) {
			return ((2.0f * (float)M_PI * f) / sr);
		}

		uint8_t waveform;
		float   amp, freq;
		float   sr, phase, phase_inc;
		float   last_out, last_freq;
	};
} // namespace daisysp
#endif
#endif
