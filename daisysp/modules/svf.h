// State Variable Filter:
// Double Sampled, Stable
// 
// Credit to Andrew Simper from musicdsp.org
// This is his "State Variable Filter (Double Sampled, Stable)"
// Additional thanks to Laurent de Soras for stability limit, and 
// Stefan Diedrichsen for the correct notch output
//
// Ported by: Stephen Hensley
//
#pragma once
#ifndef DSY_SVF_H
#define DSY_SVF_H
#ifdef __cplusplus
extern "C"
{
#endif 

typedef struct
{
	float sr, fc, res, drive, freq, damp;
	float notch, low, high, band, peak;
	float input;
	float out_low, out_high, out_band, out_peak, out_notch;
}dsy_svf_t;
// general
void dsy_svf_init(dsy_svf_t *f, float sample_rate);
void dsy_svf_process(dsy_svf_t *f, float *in);
// setters
void dsy_svf_set_fc(dsy_svf_t *f, float frequency);
void dsy_svf_set_res(dsy_svf_t *f, float res);
void dsy_svf_set_drive(dsy_svf_t *f, float drive);
// outs
float dsy_svf_notch(dsy_svf_t *f);
float dsy_svf_low(dsy_svf_t *f);
float dsy_svf_high(dsy_svf_t *f);
float dsy_svf_band(dsy_svf_t *f);
float dsy_svf_peak(dsy_svf_t *f);

#ifdef __cplusplus
}
#endif
#endif