#pragma once
#ifndef DSY_NOISE_H
#define DSY_NOISE_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
typedef struct
{
	float amp;
	int32_t randseed;
}dsy_noise_t;
void dsy_noise_init(dsy_noise_t *p);
float dsy_noise_process(dsy_noise_t *p);
#ifdef __cplusplus
}
#endif
#endif