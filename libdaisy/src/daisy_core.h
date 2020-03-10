#pragma once
#ifndef DSY_CORE_HW_H
#define DSY_CORE_HW_H
#include <stdint.h>

typedef enum
{
	DSY_GPIOA,
	DSY_GPIOB,
	DSY_GPIOC,
	DSY_GPIOD,
	DSY_GPIOE,
	DSY_GPIOF,
	DSY_GPIOG,
	DSY_GPIOH,
	DSY_GPIOI,
	DSY_GPIOJ,
	DSY_GPIOK,
	DSY_GPIOX, // This is a non-existant port for unsupported bits of hardware.
	DSY_GPIO_LAST,
}dsy_gpio_port;


typedef struct
{
	dsy_gpio_port port;
	uint8_t pin; // number 0-15
} dsy_gpio_pin;


#endif
