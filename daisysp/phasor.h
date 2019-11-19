#pragma once
#ifndef DSY_PHASOR_H
#define DSY_PHASOR_H
#ifdef __cplusplus
extern "C"
{
#endif

// Generates a normalized signal moving from 0-1 at the specified frequency.
typedef struct
{
	float freq;
	float sr, inc, phs;
}dsy_phasor_t;

// sr, and freq are in Hz
// initial phase is in radians
void dsy_phasor_init(dsy_phasor_t *p, float sr, float freq, float initial_phase);
void dsy_phasor_set_freq(dsy_phasor_t *p, float freq);
float dsy_phasor_process(dsy_phasor_t *p);

#ifdef __cplusplus
}
#endif
#endif
