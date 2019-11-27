#pragma once
#ifndef DSY_KNOB_H
#define DSY_KNOB_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
enum
{
	DSY_KNOB_THRESH_NO_LOCK,
	DSY_KNOB_THRESH_LOCK,
	DSY_KNOB_THRESH_LAST,
};
typedef struct
{
	uint16_t *raw;
	float   coeff, thresh, sr, val, prev, delta;
	float	 scale, offset;
	uint8_t lock, flip;
}dsy_ctrl;

// raw_ptr is a pointer to the adc's raw 16-bit buffer value. use dsy_adc_get_rawptr(chn)
// slewrate is the time in seconds it takes changes in the ctrl to reach their new position.
// change_thresh is the level at which a ctrl change will be detected, and unlock
// sr is used to configure the filters.
void dsy_ctrl_init(dsy_ctrl *p,
				   uint16_t *rawptr,
				   float	 scale,
				   float	 offset,
				   float	 slewrate,
				   uint8_t   change_thresh,
				   uint8_t   flip,
				   float	 sr);


// Initializes standard pot:
// 0-3v3 -> 0-1.0
// smoothing filter: 10ms smoothing
// lock threshold at 0.001 movement detection
void dsy_ctrl_init_knob(dsy_ctrl *p, uint16_t *rawptr, float sr);

// Initializes eurorack bipolar CV:
// -5V -> +5V to -1.0 -> 1.0
// smoothing filter: 2ms smoothing
// lock threshold at 0.002 movement detection
// flipped 
void dsy_ctrl_init_bipolar_cv(dsy_ctrl *p, uint16_t *rawptr, float sr);


float dsy_ctrl_process(dsy_ctrl *p);

#ifdef __cplusplus
}
#endif
#endif
