#pragma once
#ifndef DSY_OSCILLATOR_H
#define DSY_OSCILLATOR_H
#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
	DSY_OSC_WAVE_SIN,
	DSY_OSC_WAVE_TRI,
	DSY_OSC_WAVE_SAW,
	DSY_OSC_WAVE_RAMP,
	DSY_OSC_WAVE_SQUARE,
	DSY_OSC_WAVE_POLYBLEP_TRI,
	DSY_OSC_WAVE_POLYBLEP_SAW,
	DSY_OSC_WAVE_POLYBLEP_SQUARE,
	DSY_OSC_WAVE_LAST,
}dsy_oscillator_waveform;

typedef struct
{
	float freq, amp;
	dsy_oscillator_waveform wave;
	float sr, phase, phase_inc, last_out, last_freq;	
}dsy_oscillator;

void dsy_oscillator_init(dsy_oscillator *p, float sr);
float dsy_oscillator_process(dsy_oscillator *p);
void dsy_oscillator_reset_phase(dsy_oscillator *p);

#ifdef __cplusplus
}
#endif
#endif
