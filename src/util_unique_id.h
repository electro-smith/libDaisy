#pragma once
#ifndef DSY_UTIL_UNIQUE_ID_H
#define DSY_UTIL_UNIQUE_ID_H

#include "daisy_core.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** @addtogroup utility
    @{
*/


    /** Returns 96-bit Unique ID of the MCU
    @author shensley
    @date May 2020
    */

    /** fills the three pointer arguments with the unique ID of the MCU. 
    \param *w0 First pointer
    \param *w1 Second pointer
    \param *w2 Third pointer
    */
    void dsy_get_unique_id(uint32_t *w0, uint32_t *w1, uint32_t *w2);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
