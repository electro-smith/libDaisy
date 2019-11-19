#pragma once
#ifndef DSY_METRO_H
#define DSY_METRO_H
#include <stdint.h>
typedef struct
{
	float freq;
	float phs, sr, phs_inc;
} dsy_metro_t;

void dsy_metro_init(dsy_metro_t *p, float freq, float sr);

void dsy_metro_set_freq(dsy_metro_t *p, float freq);

// Returns 1 on trigger, otherwise 0
uint8_t dsy_metro_process(dsy_metro_t *p);

#endif

