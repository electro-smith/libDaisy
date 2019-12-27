#pragma once
#ifndef DSY_OSCILLATOR_H
#define DSY_OSCILLATOR_H
#include <stdint.h>
#ifdef __cplusplus
// # oscillator
// 
//      Synthesis of several waveforms, including polyBLEP bandlimited waveforms.
//
// example:
//
// ~~~~
// daisysp::oscillator osc;
// init()
// {
//     osc.init(SAMPLE_RATE);
//     osc.set_frequency(440);
//     osc.set_amp(0.25);
//     osc.set_waveform(WAVE_TRI);
// }
//
// callback(float *in, float *out, size_t size)
// {
//     for (size_t i = 0; i < size; i+=2)
//     {
//         out[i] = out[i+1] = osc.process();
//     }
// }
// ~~~~
namespace daisysp
{
	class oscillator
	{
	  public:
		oscillator() {}
		~oscillator() {}

// ## Waveforms
//
// Choices for output waveforms, POLYBLEP are appropriately labeled. Others are naive forms.
// ~~~~
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
// ~~~~

// ### init
//
// Initializes the oscillator 
//
// float samplerate - sample rate of the audio engine being run, and the frequency that the process function will be called.
//
// Defaults:
// - freq = 100 Hz
// - amp = 0.5 
// - waveform = sine wave.
//
// ~~~~
		void init(float samplerate)
// ~~~~
		{
			sr		  = samplerate;
			freq	  = 100.0f;
			amp		  = 0.5f;
			phase_	 = 0.0f;
			phase_inc = calc_phase_inc(freq);
			waveform  = WAVE_SIN;
		}

// ### set_freq
//
// Changes the frequency of the oscillator, and recalculates phase_ increment.
// ~~~~
		inline void set_freq(const float f) 
// ~~~~
		{
			freq	  = f;
			phase_inc = calc_phase_inc(f);
		}

// ### set_amp
//
// Sets the amplitude of the waveform.
// ~~~~
		inline void set_amp(const float a) { amp = a; }
// ~~~~

// ### set_waveform
//
// Sets the waveform to be synthesized by the process() function.
// ~~~~
		inline void set_waveform(const uint8_t wf) { waveform = wf < WAVE_LAST ? wf : WAVE_SIN; }
// ~~~~

// ### set_phase
//
// Sets the current phase in radians of the oscillator.
// ~~~~
		inline void set_phase(const float phase) { phase_ = phase; }
// ~~~~ 

// ### process
//
// Processes the waveform to be generated, returning one sample. This should be called once per sample period.
// ~~~~
		float process();
// ~~~~

	  private:
		float   calc_phase_inc(float f);
		uint8_t waveform;
		float   amp, freq;
		float   sr, phase_, phase_inc;
		float   last_out, last_freq;
	};
} // namespace daisysp
#endif
#endif
