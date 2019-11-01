#pragma once
#ifndef DSY_DAC_H
#define DSY_DAC_H
#include "dsy_core_hw.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
	DSY_DAC_MODE_POLLING,
	DSY_DAC_MODE_LAST,
}dsy_dac_mode;

typedef enum
{
	DSY_DAC_BITS_8,
	DSY_DAC_BITS_12,
	DSY_DAC_BITS_LAST,
}dsy_dac_bitdepth;

typedef enum
{
	DSY_DAC_CHN1,
	DSY_DAC_CHN2,
	DSY_DAC_CHN_LAST,
	DSY_DAC_CHN_BOTH,
} dsy_dac_channel;

typedef struct
{
	dsy_dac_mode mode;
	dsy_dac_bitdepth bitdepth;
	dsy_gpio_pin pin_config[DSY_DAC_CHN_LAST];
}dsy_dac_handle_t;

void dsy_dac_init(dsy_dac_handle_t *dsy_hdac, dsy_dac_channel channel);
void dsy_dac_start(dsy_dac_channel channel);
void dsy_dac_write(dsy_dac_channel channel, uint16_t val);

#ifdef __cplusplus
}
#endif
#endif
