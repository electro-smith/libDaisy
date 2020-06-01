//
//
/** Returns 96-bit Unique ID of the MCU
*/
//
//
/** **Author**: shensley
*/
//
/** **Date**: May 2020
*/
//
#pragma once
#ifndef DSY_UTIL_UNIQUE_ID_H
#define DSY_UTIL_UNIQUE_ID_H

#include "daisy_core.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** fills the three pointer arguments with the unique ID of the MCU.
*/
void dsy_get_unique_id(uint32_t *w0, uint32_t *w1, uint32_t *w2);

#ifdef __cplusplus
}
#endif

#endif
