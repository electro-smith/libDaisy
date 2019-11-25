#pragma once
#ifndef DSY_SWITCH_H
#define DSY_SWITCH_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "core.h"
#include "gpio.h"

// Usage:
// Using the dsy_switch_state(), will work with no setup other than init.
// For edge, and time based functions, you'll have to call debounce() at 
// a regular interval (i.e. 1ms)
// In order not to miss those events, the rising/falling edge checks should
// be made at the same frequency as the debounce() function.

typedef enum
{
	DSY_SWITCH_TYPE_TOGGLE,
	DSY_SWITCH_TYPE_MOMENTARY,
	DSY_SWITCH_TYPE_LAST,
} dsy_switch_type;

typedef enum
{
	DSY_SWITCH_POLARITY_NORMAL,
	DSY_SWITCH_POLARITY_INVERTED,
	DSY_SWITCH_POLARTIY_LAST,
} dsy_switch_polarity;

typedef enum
{
	DSY_SWITCH_NOPULL,
	DSY_SWITCH_PULLUP,
	DSY_SWITCH_PULLDOWN,
} dsy_switch_pull;

typedef struct
{
	dsy_switch_type type;
	dsy_switch_polarity polarity;
	dsy_switch_pull		pull;
	dsy_gpio gpio;
	dsy_gpio_pin		pin_config;
	uint8_t			state; // used for debouncing
} dsy_switch;

void dsy_switch_init(dsy_switch *sw);

void dsy_switch_debounce(dsy_switch *sw);

uint8_t dsy_switch_falling_edge(dsy_switch *sw);
uint8_t dsy_switch_rising_edge(dsy_switch *sw);
uint8_t dsy_switch_state(dsy_switch *sw);

#ifdef __cplusplus
}
#endif
#endif
