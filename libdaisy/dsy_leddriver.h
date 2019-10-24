#ifndef SA_LED_DRIVER_H
#define SA_LED_DRIVER_H
#include <stdint.h>

// TODO: Abstract the colors/led mapping out of this driver. 
// That should be left to some user configuration, and 
// the daisy family boards can have header files with all
// appropriate mappings. (i.e. dsy_patch_bsp.h)

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

typedef struct {
	uint16_t red, green, blue;
} color_t;

enum
{
	LED_A4,
	LED_A3,
	LED_A2,
	LED_A1,
	LED_B4,
	LED_B3,
	LED_B2,
	LED_B1,
	LED_C4,
	LED_C3,
	LED_C2,
	LED_C1,
	LED_D4,
	LED_D3,
	LED_D2,
	LED_D1,
	LED_LAST
};

void dsy_led_driver_init(uint8_t board);
void dsy_led_driver_update();
void dsy_led_driver_set_led( uint8_t idx, float bright);

color_t* dsy_led_driver_color_by_name(uint8_t name);


#endif
