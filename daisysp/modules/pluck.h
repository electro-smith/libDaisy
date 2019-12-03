//
// pluck
// 
// This code has been extracted from the Csound opcode "pluck"
// It has been modified to work as a Daisy Soundpipe module.
// 
// Original Author(s): Barry Vercoe, John ffitch
//
// Year: 1991
//
// Location: OOps/ugens4.c
//

#pragma once
#ifndef DSY_PLUCK_H
#define DSY_PLUCK_H

#include <stdint.h>
#ifdef __cplusplus

#define PLUCK_MODE_RECURSIVE 0
#define PLUCK_MODE_WEIGHTED_AVERAGE 1

namespace daisysp
{
	class pluck
	{
	public:
		pluck() {}
		~pluck() {}

		void init (float sample_rate, float *buf, int32_t npt, int32_t mode);
		void process (float *trig, float *out);

		// Setters
		inline void set_amp(float amp) { amp_ = amp; }
		inline void set_freq(float freq) { freq_ = freq; }
		inline void set_decay(float decay) { decay_ = decay; }
		inline void set_mode(int32_t mode) { mode_ = mode; }

		// Getters
		inline float get_amp() { return amp_; }
		inline float get_freq() { return freq_; }
		inline float get_decay() { return decay_; }
		inline int32_t get_mode() { return mode_; }

	private:
		void reinit();
		float amp_, freq_, decay_, damp_, ifreq_;
	    float sicps_;
	    int32_t phs256_, npts_, maxpts_;
	    float *buf_;
	    float sample_rate_;
	    char init_;
	    int32_t mode_;
	};
} // namespace daisysp
#endif
#endif
