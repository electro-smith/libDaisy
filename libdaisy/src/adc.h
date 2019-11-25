#pragma once
#ifndef DSY_ADC_H
#define DSY_ADC_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include <stdlib.h>
#include "core.h"

	// Limitations:
	// - For now speed is fixed at ASYNC_DIV128 for ADC Clock, and SAMPLETIME_64CYCLES_5 for each conversion.
	// - No OPAMP config for the weird channel
	// - No oversampling built in yet

	typedef enum
	{
		DSY_ADC_PIN_CHN3,
		DSY_ADC_PIN_CHN4,
		DSY_ADC_PIN_CHN5,
		DSY_ADC_PIN_CHN7,
		DSY_ADC_PIN_CHN10,
		DSY_ADC_PIN_CHN11,
		DSY_ADC_PIN_CHN15,
		DSY_ADC_PIN_CHN16,
		DSY_ADC_PIN_CHN17,
		DSY_ADC_PIN_CHN18,
		DSY_ADC_PIN_CHN19,
		DSY_ADC_PIN_LAST,
	} dsy_adc_pin;

	typedef enum
	{
		MUX_SEL_0,
		MUX_SEL_1,
		MUX_SEL_2,
		MUX_SEL_LAST,
	} dsy_adc_mux_pin;

	typedef struct
	{
		dsy_gpio_pin pin_config[DSY_ADC_PIN_LAST];
		dsy_gpio_pin mux_pin_config[DSY_ADC_PIN_LAST][MUX_SEL_LAST];
		uint8_t		 active_channels[DSY_ADC_PIN_LAST];
		uint8_t		 channels, mux_channels[DSY_ADC_PIN_LAST];
	} dsy_adc_handle;

	void	 dsy_adc_init(dsy_adc_handle *dsy_hadc);
	void	 dsy_adc_start();
	void	 dsy_adc_stop();
	uint16_t dsy_adc_get(uint8_t chn);
	float	dsy_adc_get_float(uint8_t chn);

	// These are getters for multiplexed inputs on a single channel (up to 8 per ADC input).
	uint16_t dsy_adc_get_mux(uint8_t chn, uint8_t idx);
	float	dsy_adc_get_mux_float(uint8_t chn, uint8_t idx);

#ifdef __cplusplus
}
#endif
#endif // DSY_ADC_H