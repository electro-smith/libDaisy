#pragma once
#ifndef DSY_CORE_HW_H
#define DSY_CORE_HW_H
#include <stdint.h>
#include "stm32h7xx_hal.h"
		 

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


// Maps Daisy interface to STM32 HAL --
// I'd like to get all of this stuff tucked away somewhere inbetween the HAL, and User level
// So that I can start to slowly replace HAL stuff over time.
const static GPIO_TypeDef* gpio_hal_port_map[DSY_GPIO_LAST] = {
	GPIOA,
	GPIOB,
	GPIOC,
	GPIOD,
	GPIOE,
	GPIOF,
	GPIOG,
	GPIOH,
	GPIOI,
	GPIOJ,
	GPIOK,
	NULL,
};

const static uint16_t gpio_hal_pin_map[16] = {
	GPIO_PIN_0,
	GPIO_PIN_1,
	GPIO_PIN_2,
	GPIO_PIN_3,
	GPIO_PIN_4,
	GPIO_PIN_5,
	GPIO_PIN_6,
	GPIO_PIN_7,
	GPIO_PIN_8,
	GPIO_PIN_9,
	GPIO_PIN_10,
	GPIO_PIN_11,
	GPIO_PIN_12,
	GPIO_PIN_13,
	GPIO_PIN_14,
	GPIO_PIN_15,
};

#endif
