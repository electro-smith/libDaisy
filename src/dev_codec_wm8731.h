#ifndef SA_DRIVERS_CODEC_H_
#define SA_DRIVERS_CODEC_H_

#include <stddef.h>
#include "per_sai.h"
#include "per_i2c.h"

uint8_t codec_wm8731_init(dsy_i2c_handle *hi2c,
						  uint8_t			 mcu_is_master,
						  int32_t			 sample_rate, 
						  uint8_t			 bitdepth);
uint8_t codec_wm8731_enter_bypass(dsy_i2c_handle *hi2c);
uint8_t codec_wm8731_exit_bypass(dsy_i2c_handle *hi2c);

#endif
