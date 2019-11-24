#pragma once
#ifndef DSYSP_REVERBSC_H
#define DSYSP_REVERBSC_H
#ifdef __cplusplus
extern "C"
{
#endif

#define DSY_REVERBSC_MAX_SIZE 32767
typedef struct
{
	int     writePos;
	int     bufferSize;
	int     readPos;
	int     readPosFrac;
	int     readPosFrac_inc;
	int     dummy;
	int     seedVal;
	int     randLine_cnt;
	float filterState;
	float *buf;
}dsy_reverbsc_dl_t;
typedef struct
{
	float feedback, lpfreq;
	float iSampleRate, iPitchMod, iSkipInit;
	float sampleRate;
	float dampFact;
	float prv_LPFreq;
	int initDone;
	dsy_reverbsc_dl_t delayLines[8];
	float aux[DSY_REVERBSC_MAX_SIZE];
}dsy_reverbsc_t;

int dsy_reverbsc_init(dsy_reverbsc_t *p, float sr);
int dsy_reverbsc_process(dsy_reverbsc_t *p, float *in1, float *in2, float *out1, float *out2);
#ifdef __cplusplus
}
#endif
#endif

