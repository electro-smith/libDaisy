// Attack Decay Envelope
// By shensley
//
// Trigger-able envelope with adjustable min/max, and independent per-segment time control.
// 
// TODO:
// - Add Cycling
// - Implement Curve (its only linear for now).
// - Maybe make this an ADSR that has AD/AR/ASR modes.
//
#pragma once
#ifndef DSY_ADENV_H
#define DSY_ADENV_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
typedef enum {
	DSY_ADENV_SEG_IDLE,
	DSY_ADENV_SEG_RISE,
	DSY_ADENV_SEG_FALL,
	DSY_ADENV_SEG_LAST,
} dsy_adenv_segment;

typedef struct {
	dsy_adenv_segment current_segment;
	float segment_time[DSY_ADENV_SEG_LAST];
	float sr, phase_inc, min, max, multiplier, output, curve_scalar;
	uint32_t phase;
	uint8_t trigger;
} dsy_adenv;

void dsy_adenv_init(dsy_adenv *p, float sr);
void dsy_adenvrigger(dsy_adenv *p);
void dsy_adenv_set_segment_time(dsy_adenv *p, dsy_adenv_segment seg, float time);
void dsy_adenv_set_curve_scalar(dsy_adenv *p, float scalar);
void dsy_adenv_set_min_max(dsy_adenv *p, float min, float max);
float dsy_adenv_process(dsy_adenv *p);


#ifdef __cplusplus
}
#endif
#endif
