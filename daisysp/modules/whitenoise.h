#pragma once
#ifndef DSY_WHITENOISE_H
#define DSY_WHITENOISE_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
typedef struct
{
	float amp;
	int32_t randseed;
}dsy_whitenoise;
void dsy_whitenoise_init(dsy_whitenoise *p);
float dsy_whitenoise_process(dsy_whitenoise *p);
#ifdef __cplusplus
}
#endif
#endif
