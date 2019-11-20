// Crossfade
// Performs a linear crossfade between two signals
// 
// Original author: Paul Batchelor
//
// Ported from Soundpipe by Andrew Ikenberry

#pragma once
#ifndef DSY_SVF_H
#define DSY_SVF_H
#ifdef __cplusplus
extern "C"
{
#endif 

typedef struct
{
	float pos, in1, in2, out;

} dsy_crossfade;

// initialization
void dsy_crossfade_init(dsy_crossfade *c);

// processing
float dsy_crossfade_process(dsy_crossfade *c, float *in1, float *in2);

// setter
void dsy_crossfade_set_pos(dsy_crossfade *c, float pos);

#ifdef __cplusplus
}
#endif
#endif