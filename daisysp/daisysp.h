//	DaisySP is a DSP Library targeted at the Electrosmith Daisy Product Line.
//  Author: Stephen Hensley, 2019
//
//	However, this is decoupled from the hardware in such a way that it 
//		should be useful outside of the ARM context with different build configurations.
//	
//	A few general notes about the contents of the library:
// 		- all types will end with '_t'
//		- all memory usage is static. 
//		- in cases of potentially large memory usage, the user will supply a buffer and a size.
// 		- all blocks will have an _init() function, and a _process() function.
//		- all blocks, unless otherwise noted, will process a single sample at a time.
//		- all processing will be done with 'float' type unless otherwise noted.	
//		- a common dsysp_data_t type will be used for many init() functions to reduce user code complexity
// 
#pragma once
#ifndef DSYSP_H
#define DSYSP_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef FORCE_INLINE
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
#endif

typedef struct
{
	float samplerate;
	float channels;
}dsysp_data_t;

FORCE_INLINE float dsysp_clip(float in, float min, float max)
{
	if (in < min)
		return min;
	else if (in > max)
		return max;
	else
		return in;
}

// Listed in Alphabetical Order
// Each of the below has/should have an example project.
#include "dsy_adenv.h"
#include "dsy_decimator.h"
#include "dsy_nlfilt.h"
#include "dsy_metro.h"
#include "dsy_noise.h"
#include "dsy_oscillator.h"
#include "dsy_phasor.h"
#include "dsy_reverbsc.h"
#include "dsy_svf.h"


#ifdef __cplusplus
}
#endif 
#endif // DSYSP_H
