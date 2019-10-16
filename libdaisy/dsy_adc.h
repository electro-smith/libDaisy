#pragma once
#ifndef DSY_ADC_H
#define DSY_ADC_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include <stdlib.h>

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

void	 dsy_adc_init(uint8_t board);
void	 dsy_adc_start();
void	 dsy_adc_stop();
uint16_t dsy_adc_get(uint8_t chn);
float	dsy_adc_get_float(uint8_t chn);

#ifdef __cplusplus
}
#endif
#endif // DSY_ADC_H