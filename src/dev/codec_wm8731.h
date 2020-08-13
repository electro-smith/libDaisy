#ifndef SA_DRIVERS_CODEC_H_
#define SA_DRIVERS_CODEC_H_

#include <stddef.h>
#include "per/sai.h"
#include "per/i2c.h"

/** 
    @brief Driver for the WM8731 Codec
*/

/** @addtogroup codec
    @{
*/

/** 
    Resets the WM8731
    \param *hi2c array of pins handling i2c?
    \param mcu_is_master &
    \param sample_rate Sample rate to run codec at
    \param bitdepth Bit depth to run codec at
*/
uint8_t codec_wm8731_init(dsy_i2c_handle *hi2c,
                          uint8_t         mcu_is_master,
                          int32_t         sample_rate,
                          uint8_t         bitdepth);

/**
   Put codec into bypass mode
   \param *hi2c pins handling i2c
*/
uint8_t codec_wm8731_enter_bypass(dsy_i2c_handle *hi2c);

/**
   Take codec out of bypass mode
   \param *hi2c pins handling i2c
*/
uint8_t codec_wm8731_exit_bypass(dsy_i2c_handle *hi2c);

#endif
/** @} */
