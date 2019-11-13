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
} adenv_segment_t;

typedef struct {
	adenv_segment_t current_segment;
	float segment_time[DSY_ADENV_SEG_LAST];
	float sr, phase_inc, min, max, multiplier, output, curve_scalar;
	uint32_t phase;
	uint8_t trigger;
} dsy_adenv_t;

void dsy_adenv_init(dsy_adenv_t *p, float sr);
void dsy_adenv_trigger(dsy_adenv_t *p);
void dsy_adenv_set_segment_time(dsy_adenv_t *p, adenv_segment_t seg, float time);
void dsy_adenv_set_curve_scalar(dsy_adenv_t *p, float scalar);
void dsy_adenv_set_min_max(dsy_adenv_t *p, float min, float max);
float dsy_adenv_process(dsy_adenv_t *p);


#ifdef __cplusplus
}
#endif
#endif
