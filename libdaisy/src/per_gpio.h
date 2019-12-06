#pragma once
#ifndef DSY_GPIO_H
#define DSY_GPIO_H
#include "daisy_core.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
	DSY_GPIO_MODE_INPUT,
	DSY_GPIO_MODE_OUTPUT_PP,
	DSY_GPIO_MODE_OUTPUT_OD,
	DSY_GPIO_MODE_LAST,
} dsy_gpio_mode;

typedef enum
{
	DSY_GPIO_NOPULL,
	DSY_GPIO_PULLUP,
	DSY_GPIO_PULLDOWN,
} dsy_gpio_pull;

typedef struct
{
	dsy_gpio_pin pin;
	dsy_gpio_mode mode;
	dsy_gpio_pull pull;
} dsy_gpio;

void dsy_gpio_init(dsy_gpio *p);
void dsy_gpio_deinit(dsy_gpio *p);
uint8_t dsy_gpio_read(dsy_gpio *p);
void dsy_gpio_write(dsy_gpio *p, uint8_t state);
void dsy_gpio_toggle(dsy_gpio *p);

#ifdef __cplusplus
}
#endif

#endif
