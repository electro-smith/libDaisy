// # GPIO
// ## Description
// General Purpose IO driver 
#pragma once
#ifndef DSY_GPIO_H
#define DSY_GPIO_H
#include "daisy_core.h"
#ifdef __cplusplus
extern "C"
{
#endif

// ## Data
// ### Gpio Mode
// Sets the mode of the GPIO 
// ~~~~
typedef enum
{
	DSY_GPIO_MODE_INPUT,
	DSY_GPIO_MODE_OUTPUT_PP, // Push-Pull
	DSY_GPIO_MODE_OUTPUT_OD, // Open-Drain
    DSY_GPIO_MODE_ANALOG,
	DSY_GPIO_MODE_LAST,
} dsy_gpio_mode;
// ~~~~

// ### Pull
// Configures whether an internal Pull up
//  or Pull down resistor is used
// ~~~~
typedef enum
{
    DSY_GPIO_NOPULL,
    DSY_GPIO_PULLUP,
    DSY_GPIO_PULLDOWN,
} dsy_gpio_pull;
// ~~~~

// ### dsy_gpio
// Struct for holding the pin, and configuration
// ~~~~
typedef struct
{
	dsy_gpio_pin pin;
	dsy_gpio_mode mode;
	dsy_gpio_pull pull;
} dsy_gpio;
// ~~~~

// ## Functions
// ### init
// Initializes the gpio with the settings configured.
// ~~~~
void dsy_gpio_init(const dsy_gpio *p);
// ~~~~

// ### deinit
// Deinitializes the gpio pin
// ~~~~
void dsy_gpio_deinit(const dsy_gpio *p);
// ~~~~

// ### read
// Reads the state of the gpio pin
//
// returning 1 if the pin is HIGH, and 0 if the pin is LOW
// ~~~~
uint8_t dsy_gpio_read(const dsy_gpio *p);
// ~~~~

// ### write
// Writes the state to the gpio pin
//
// Pin will be set to 3v3 when state is >0, and 0V when state is 0
// ~~~~
void dsy_gpio_write(const dsy_gpio *p, uint8_t state);
// ~~~~

// ### toggle
// Toggles the state of the pin so that it is not at the same
//		state as it was previously.
// ~~~~
void dsy_gpio_toggle(const dsy_gpio *p);
// ~~~~
#ifdef __cplusplus
}
#endif

#endif
