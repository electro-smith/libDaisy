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


// Always include
#include "dsy_system.h" 
#include "dsy_core_hw.h"

//enum
//{
//	DSY_SYS_BOARD_DAISY,
//	DSY_SYS_BOARD_DAISY_SEED,
//	DSY_SYS_BOARD_AUDIO_BB,
//	DSY_SYS_BOARD_EURORACK_TESTER,
//	DSY_SYS_BOARD_LAST,
//};


// TODO: Maybe move these into separate libdaisy_conf.h file that can be added to user projects.
#define DSY_USE_QSPI_DRV
#define DSY_USE_SDRAM_DRV
#define DSY_USE_AUDIO_DRV
#define DSY_USE_ADC_DRV
#define DSY_USE_DAC_DRV
#define DSY_USE_LEDDRIVER_DRV


#ifdef DSY_USE_QSPI_DRV
#include "dsy_qspi.h"
#endif
#ifdef DSY_USE_SDRAM_DRV
#include "dsy_sdram.h"
#endif
#ifdef DSY_USE_AUDIO_DRV
#include "dsy_audio.h"
#endif
#ifdef DSY_USE_ADC_DRV
#include "dsy_adc.h"
#endif
#ifdef DSY_USE_DAC_DRV
#include "dsy_dac.h"
#endif
#ifdef DSY_USE_LEDDRIVER_DRV
#include "dsy_leddriver.h"
#endif

#include "dsy_sai.h"
#include "dsy_i2c.h"
#include "dsy_gpio.h"
#include "dsy_switch.h"

#endif
