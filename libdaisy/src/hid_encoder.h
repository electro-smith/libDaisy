#pragma once
#ifndef DSY_ENCODER_H
#define DSY_ENCODER_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "daisy_core.h"
#include "per_gpio.h"
enum
{
	DSY_ENCODER_PIN_A,
	DSY_ENCODER_PIN_B,
	DSY_ENCODER_PIN_CLICK,
	DSY_ENCODER_PIN_LAST,
};
typedef struct
{
	dsy_gpio_pin pin_config[DSY_ENCODER_PIN_LAST];
	dsy_gpio   pins[DSY_ENCODER_PIN_LAST];
	uint32_t	 time;
	uint8_t		 states[DSY_ENCODER_PIN_LAST];
	int32_t		 inc;
} dsy_encoder;
void dsy_encoder_init(dsy_encoder *p);
void dsy_encoder_debounce(dsy_encoder *p);
int32_t dsy_encoder_inc(dsy_encoder *p);
uint8_t dsy_encoder_state(dsy_encoder *p);
uint8_t dsy_encoder_rising_edge(dsy_encoder *p);
uint8_t dsy_encoder_falling_edge(dsy_encoder *p);
uint32_t dsy_encoder_time_held(dsy_encoder *p);

#ifdef __cplusplus
}
#endif
#endif
