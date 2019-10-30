#pragma once
#ifndef DSY_ADC_H
#define DSY_ADC_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include <stdlib.h>
#include "dsy_core_hw.h"

	// For now this Initializes all 8 of the specific ADC pins for Daisy Seed.
	// I'd like to make the following things easily configurable:
	// - Selecting which channels should be initialized/included in the sequence conversion.
	// - Setup a similar start function for an external mux, but that seems outside the scope of this file.

	// Limitations:
	// - For now speed is fixed at ASYNC_DIV128 for ADC Clock, and SAMPLETIME_64CYCLES_5 for each conversion.
	// - Only Daisy Seed GPIO init is set up.
	// - All 8 ADC channels are on whether you like it or not
	// - No OPAMP config for the weird channel
	// - No oversampling built in

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

	typedef struct
	{
		dsy_gpio_pin pin_config[DSY_ADC_PIN_LAST];
		uint8_t		 active_channels[DSY_ADC_PIN_LAST]; 
		uint8_t		 channels;
	} dsy_adc_handle_t;

	void	 dsy_adc_init(dsy_adc_handle_t *dsy_hadc);
	void	 dsy_adc_start();
	void	 dsy_adc_stop();
	uint16_t dsy_adc_get(uint8_t chn);
	float	dsy_adc_get_float(uint8_t chn);

#ifdef __cplusplus
}
#endif
#endif // DSY_ADC_H