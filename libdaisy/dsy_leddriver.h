#pragma once
#ifndef SA_LED_DRIVER_H
#define SA_LED_DRIVER_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include "dsy_i2c.h"

	// TODO: Abstract the colors of this driver.

	enum
	{
		LED_COLOR_RED,
		LED_COLOR_GREEN,
		LED_COLOR_BLUE,
		LED_COLOR_WHITE,
		LED_COLOR_PURPLE,
		LED_COLOR_CYAN,
		LED_COLOR_GOLD,
		LED_COLOR_OFF,
		LED_COLOR_LAST
	};

	typedef struct
	{
		uint16_t red, green, blue;
	} color_t;

	void	 dsy_led_driver_init(dsy_i2c_handle_t *dsy_i2c);
	void	 dsy_led_driver_update();
	void	 dsy_led_driver_set_led(uint8_t idx, float bright);
	color_t* dsy_led_driver_color_by_name(uint8_t name);

#ifdef __cplusplus
}
#endif
#endif
