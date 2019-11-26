#pragma once
#ifndef DSY_KNOB_H
#define DSY_KNOB_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
	typedef enum
	{
		DSY_KNOB_THRESH_NO_LOCK,
		DSY_KNOB_THRESH_LOCK,
		DSY_KNOB_THRESH_LAST,
	}dsy_knob_thresh;
	typedef struct
	{
		uint16_t *raw;
		float   coeff, thresh, sr, val, prev, delta;
		uint8_t lock;
	}dsy_knob;

	// raw_ptr is a pointer to the adc's raw 16-bit buffer value. use dsy_adc_get_rawptr(chn)
	// slewrate is the time in seconds it takes changes in the knob to reach their new position.
	// change_thresh is the level at which a knob change will be detected, and unlock
	// sr is used to configure the filters.
	void dsy_knob_init(dsy_knob *p,
					   uint16_t *rawptr,
					   float	 slewrate,
					   dsy_knob_thresh change_thresh, 
					   float	 sr);

	float dsy_knob_process(dsy_knob *p);
#ifdef __cplusplus
}
#endif
#endif
