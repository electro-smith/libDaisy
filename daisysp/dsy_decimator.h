#pragma once
#ifndef DSYSP_DECIMATOR_H
#define DSYSP_DECIMATOR_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

typedef struct
{
	// interface data
	float downsample_factor, bitcrush_factor;
	// internal data
	float downsampled, bitcrushed;
	uint32_t inc, threshold;
}dsy_decimator_t;

void dsy_decimator_init(dsy_decimator_t *d);
float dsy_decimator_process(dsy_decimator_t *d, float input);

#ifdef __cplusplus
}
#endif
#endif
