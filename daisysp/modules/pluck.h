//
// pluck
// 
// This code has been extracted from the Csound opcode "pluck"
// It has been modified to work as a Daisy SOundpipe module.
// 
// Original Author(s): Barry Vercoe, John ffitch
// Year: 1991
// Location: OOps/ugens4.c
//
// 

#pragma once
#ifndef PLUCK_H
#define PLUCK_H
#define PLUCK_MODE_RECURSIVE 0
#define PLUCK_MODE_WEIGHTED_AVERAGE 1

#include <stdint.h>
#ifdef __cplusplus

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
		inline void set_amp(float amp) { _amp = amp; }
		inline void set_freq(float freq) { _freq = freq; }
		inline void set_decay(float decay) { _decay = decay; }
		inline void set_sample_rate(float sample_rate) { _sample_rate = sample_rate; }
		inline void set_mode(int32_t mode) { _mode = mode; }

		// Getters
		inline float get_amp() { return _amp; }
		inline float get_freq() { return _freq; }
		inline float get_decay() { return _decay; }
		inline float get_sample_rate() { return _sample_rate; }
		inline int32_t get_mode() { return _mode; }

	private:
		void reinit();
		float _amp, _freq, _decay, _damp, _ifreq;
	    float _sicps;
	    int32_t _phs256, _npts, _maxpts;
	    float *_buf;
	    float _sample_rate;
	    char _init;
	    int32_t _mode;
/*
typedef struct {
    float amp, freq, decay, damp, ifreq;
    float sicps;
    int32_t phs256, npts, maxpts;
    float *buf;
    float sr;
    char init;
    int32_t mode;
} daisy_pluck;

void daisy_pluck_init(daisy_pluck *p, float sr, float *buf, int32_t npt, int32_t mode);
void daisy_pluck_compute(daisy_pluck *p, float *trig, float *out);
*/
	};
} // namespace daisysp
#endif
#endif
