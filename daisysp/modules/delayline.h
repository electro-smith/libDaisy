// Simple Delay Line
// November 2019
// By: shensley
//
// User manages their own buffer in order to provide 
//     flexibility without dynamic memory.
//
// TODO: add some sort of type flexibility -- for now its just floats.
#pragma once
#ifndef DSY_DELAY_H
#define DSY_DELAY_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdlib.h>

typedef struct
{
	uint8_t interp;		
	size_t write_ptr, delay, size;
	float delay_frac, delay_sec, sr;
	float *line;
}dsy_delayline;

void dsy_delay_init(dsy_delayline *p, float* buff, size_t buff_size, float samplerate);
void dsy_delay_reset(dsy_delayline *p);
void dsy_delay_set_delay_samps(dsy_delayline *p, size_t size);
void dsy_delay_set_delay_sec(dsy_delayline *p, float sec);
void dsy_delay_line_write(dsy_delayline *p, float val);
float dsy_delay_line_read(dsy_delayline *p);

#ifdef __cplusplus
}
#endif
#endif
