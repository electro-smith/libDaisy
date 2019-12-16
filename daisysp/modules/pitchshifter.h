// # pitchshift
// From ucsd.edu "Pitch Shifting"
// t = 1 - ((s *f) / R)
// where:
// s is the size of the delay
// f is the frequency of the lfo
// r is the samplerate
// solving for t = 12.0
// f = (12 - 1) * 48000 / SHIFT_BUFFER_SIZE;
#pragma once
#ifndef DSY_PITCHSHIFTER_H
#define DSY_PITCHSHIFTER_H
#include <stdint.h>
#include <cmath>
#ifdef USE_ARM_DSP
#include "arm_math.h"
#endif
#include "daisysp.h"

using namespace daisysp;

// Shift can be 30-100 ms lets just start with 50 for now.
// 0.050 * SR = 2400 samples (at 48kHz)
#define SHIFT_BUFFER_SIZE 4800
//#define SHIFT_BUFFER_SIZE 12000

class pitchshifter
{
public:
	pitchshifter() {}
	~pitchshifter() {}

	void init(float sr)
	{
		sr_ = sr;
		mod_freq_ = 5.0f;	
		set_semitones();
		for (uint8_t i = 0; i < 2; i++)
		{
			gain_[i] = 0.0f;	
			d_[i].init();
			phs_[i].init(sr, 50, i == 0 ? 0 : (float)M_PI);
		}
		shift_up_ = true;
	}
	float process(float &in)
	{
		float val, fade1, fade2;
		// First process delay mod/crossfade
		fade1 = phs_[0].process();
		fade2 = phs_[1].process();
		if (shift_up_)
		{
			fade1 = 1.0f - fade1;	
			fade2 = 1.0f - fade2;	
		}
		mod_[0] = fade1 * (SHIFT_BUFFER_SIZE - 1);
		mod_[1] = fade2 * (SHIFT_BUFFER_SIZE - 1);
#ifdef USE_ARM_DSP
		gain_[0] = arm_sin_f32(fade1 * (float)M_PI);
		gain_[1] = arm_sin_f32(fade2 * (float)M_PI);
#else
		gain_[0] = sinf(fade1 * (float)M_PI);
		gain_[1] = sinf(fade2 * (float)M_PI);
#endif
		
		// Handle Delay Writing
		d_[0].write(in);
		d_[1].write(in);
		// Modulate Delay Lines
		d_[0].set_delay(mod_[0]);
		d_[1].set_delay(mod_[1]);
		val = 0.0f;
		val += (d_[0].read() * gain_[0]);
		val += (d_[1].read() * gain_[1]);
		return val;
	}
	void set_transposition(const float &transpose)
	{
		float ratio;
		uint8_t idx;
		if (transpose_ != transpose)
		{
			transpose_ = transpose;
			idx = fabsf(transpose);
			ratio = semitone_ratios_[idx % 12];
			ratio *= (uint8_t)(fabsf(transpose) / 12) + 1;
			if (transpose > 0.0f)
			{
				shift_up_ = true;	
			}
			else
			{
				shift_up_ = false;	
			}
			mod_freq_ = ((ratio - 1.0f) * sr_) / SHIFT_BUFFER_SIZE;
			if (mod_freq_ < 0.0f)
			{
				mod_freq_ = 0.0f;
			}
			phs_[0].set_freq(mod_freq_);
			phs_[1].set_freq(mod_freq_);
		}
	}

private:
	inline void set_semitones()
	{
		for (size_t i = 0; i < 12; i++)
		{
			semitone_ratios_[i] = powf(2.0f, (float)i / 12);	
		}
	}
	typedef delayline<float, SHIFT_BUFFER_SIZE> shift_delay;
	shift_delay d_[2];
	float pitch_shift_, mod_freq_;
	// lfo stuff
	float sr_;
	bool shift_up_;
	phasor phs_[2];
	float gain_[2], mod_[2], transpose_;
	// pitch stuff
	float semitone_ratios_[12];
};

#endif
