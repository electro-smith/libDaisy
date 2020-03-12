#ifndef DSY_LIBDAISY_H
#define DSY_LIBDAISY_H

#include<stdint.h>

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#elif defined(__clang__)
#define FORCE_INLINE inline __attribute__((always_inline))
#pragma clang diagnostic ignored "-Wduplicate-decl-specifier"
#elif defined(__GNUC__)
#define FORCE_INLINE inline __attribute__((always_inline))
#else
#error unknown compiler
#endif

#define FBIPMAX 0.999985f	//close to 1.0f-LSB at 16 bit
#define FBIPMIN (-FBIPMAX)
#define S162F_SCALE 3.0517578125e-05f
#define F2S16_SCALE 32767.0f
#define F2S24_SCALE 8388608.0f
#define S242F_SCALE 1.192092896e-07f
#define S24SIGN 0x800000

FORCE_INLINE float s162f(int16_t x)
{
	return (float)x * S162F_SCALE;
}

FORCE_INLINE int16_t f2s16(float x)
{
	x = x <= FBIPMIN ? FBIPMIN : x;
	x = x >= FBIPMAX ? FBIPMAX : x;
	return (int32_t)(x * F2S16_SCALE);
}

FORCE_INLINE float s242f(int32_t x)
{
	x = (x ^ S24SIGN) - S24SIGN; //sign extend aka ((x<<8)>>8)
	return (float)x * S242F_SCALE;
}
FORCE_INLINE int32_t f2s24(float x) 
{
	x = x <= FBIPMIN ? FBIPMIN : x;
	x = x >= FBIPMAX ? FBIPMAX : x;
	return (int32_t)(x * F2S24_SCALE);
}

FORCE_INLINE float cube(float x) 
{
	return (x * x) * x;
}

#include "daisy_core.h"
#include "sys_system.h" 
#include "per_qspi.h"
#include "per_adc.h"
#include "per_dac.h"
#include "per_gpio.h"
#include "per_i2c.h"
#include "per_sai.h"
#include "per_tim.h"
#include "dev_leddriver.h"
#include "dev_sdram.h"
#include "dev_sr_4021.h"
#include "hid_audio.h"
#ifdef __cplusplus
#include "per_uart.h"
#include "hid_midi.h"
#include "hid_encoder.h"
#include "hid_switch.h"
#include "hid_ctrl.h"
#include "parameter.h"
#endif
#endif
