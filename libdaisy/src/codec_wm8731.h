// WM8371 Codec support.
#ifndef SA_DRIVERS_CODEC_H_
#define SA_DRIVERS_CODEC_H_

#include <stddef.h>
#include "dsy_sai.h"
#include "dsy_i2c.h"

uint8_t codec_wm8731_init(I2C_HandleTypeDef *hi2c,
						  uint8_t			 mcu_is_master,
						  int32_t			 sample_rate, 
						  uint8_t			 bitdepth);
uint8_t codec_wm8731_enter_bypass(I2C_HandleTypeDef *hi2c);
uint8_t codec_wm8731_exit_bypass(I2C_HandleTypeDef *hi2c);

#endif
