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

#include "dsy_system.h" // Always include

enum
{
	DSY_SYS_BOARD_DAISY,
	DSY_SYS_BOARD_DAISY_SEED,
	DSY_SYS_BOARD_AUDIO_BB,
	DSY_SYS_BOARD_EURORACK_TESTER,
	DSY_SYS_BOARD_LAST,
};

// TODO: Maybe move these into separate libdaisy_conf.h file that can be added to user projects.
#define DSY_USE_QSPI_DRV
#define DSY_USE_SDRAM_DRV
#define DSY_USE_AUDIO_DRV


#ifdef DSY_USE_QSPI_DRV
#include "dsy_qspi.h"
#endif
#ifdef DSY_USE_SDRAM_DRV
#include "dsy_sdram.h"
#endif
#ifdef DSY_USE_AUDIO_DRV
#include "dsy_audio.h"
#endif

#endif
