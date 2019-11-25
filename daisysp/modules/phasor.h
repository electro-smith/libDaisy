// For now this Initializes all 8 of the specific ADC pins for Daisy Seed.
// I'd like to make the following things easily configurable:
// - Selecting which channels should be initialized/included in the sequence conversion.
// - Setup a similar start function for an external mux, but that seems outside the scope of this file.
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
}dsy_phasor;

// sr, and freq are in Hz
// initial phase is in radians
void dsy_phasor_init(dsy_phasor *p, float sr, float freq, float initial_phase);
void dsy_phasor_set_freq(dsy_phasor *p, float freq);
float dsy_phasor_process(dsy_phasor *p);

#ifdef __cplusplus
}
#endif
#endif
