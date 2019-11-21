// Crossfade
// Performs a crossfade between two signals
// 
// Original author: Paul Batchelor
//
// Ported from Soundpipe by Andrew Ikenberry
// added curve option for constant power, etc.

#pragma once
#ifndef DSY_CROSSFADE_H
#define DSY_CROSSFADE_H
#ifdef __cplusplus
extern "C"
{
#endif 
#include <stdint.h>

enum 
{
	DSY_CROSSFADE_LINEAR,
	DSY_CROSSFADE_CONST_POWER,
	DSY_CROSSFADE_LAST,
};

typedef struct
{
	float pos;
	uint8_t curve;
} dsy_crossfade;

// initialization
void dsy_crossfade_init(dsy_crossfade *p, uint8_t curve);

// processing
float dsy_crossfade_process(dsy_crossfade *p, float *in1, float *in2);

// set position between two signals. range: 0-1
void dsy_crossfade_set_pos(dsy_crossfade *p, float pos);

// set curve of crossfade.
void dsy_crossfade_set_curve(dsy_crossfade *p, uint8_t curve);

#ifdef __cplusplus
}
#endif
#endif