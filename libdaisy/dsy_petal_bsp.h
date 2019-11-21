#pragma once
#ifndef DSY_PETAL_H
#define DSY_PETAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "dsy_seed.h"

#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE
#endif

// Hardware related defines.
// Switches
#define SW_1_PORT seed_ports[9]
#define SW_1_PIN seed_pins[9]
#define SW_2_PORT seed_ports[10]
#define SW_2_PIN seed_pins[10]
#define SW_3_PORT seed_ports[11]
#define SW_3_PIN seed_pins[11]
#define SW_4_PORT seed_ports[14]
#define SW_4_PIN seed_pins[14]
#define SW_5_PORT seed_ports[0]
#define SW_5_PIN seed_pins[0]
#define SW_6_PORT seed_ports[26]
#define SW_6_PIN seed_pins[26]
#define SW_7_PORT seed_ports[8]
#define SW_7_PIN seed_pins[8]

// Encoder
#define ENC_A_PORT seed_ports[28];
#define ENC_A_PIN seed_pins[28];
#define ENC_B_PORT seed_ports[27];
#define ENC_B_PIN seed_pins[27];
#define ENC_CLICK_PORT seed_ports[15];
#define ENC_CLICK_PIN seed_pins[15];

// Extra Peripherals
#define LED_DRIVER_I2C i2c1_handle

// enums for controls, etc.
enum
{
	SW_1, // Footswitch
	SW_2, // Footswitch
	SW_3, // Footswitch
	SW_4, // Footswitch
	SW_5, // Toggle
	SW_6, // Toggle
	SW_7, // Toggle
	SW_LAST,
};

// All knobs connect to ADC1_INP10 via CD4051 mux
enum
{
	KNOB_1,
	KNOB_2,
	KNOB_3,
	KNOB_4,
	KNOB_5,
	KNOB_6,
	KNOB_LAST,
};

enum
{
	CV_EXPRESSION,
	CV_LAST,
};

enum
{
	LED_RING_1_R,
	LED_RING_1_G,
	LED_RING_1_B,
	LED_RING_5_R,
	LED_RING_5_G,
	LED_RING_5_B,
	LED_RING_2_R,
	LED_RING_2_G,
	LED_RING_2_B,
	LED_RING_6_R,
	LED_RING_6_G,
	LED_RING_6_B,
	LED_RING_3_R,
	LED_RING_3_G,
	LED_RING_3_B,
	LED_FS_1,
	LED_RING_4_R,
	LED_RING_4_G,
	LED_RING_4_B,
	LED_RING_7_R,
	LED_RING_7_G,
	LED_RING_7_B,
	LED_RING_8_R,
	LED_RING_8_G,
	LED_RING_8_B,
	LED_FS_2,
	LED_FS_3,
	LED_FS_4,
	LED_FAKE1,
	LED_FAKE2,
	LED_FAKE3,
	LED_FAKE4,
	LED_LAST,
};

typedef struct
{
	daisy_handle seed;
	dsy_switch_t switches[SW_LAST];
	float knobs[KNOB_LAST];
	float cvs[CV_LAST];
} daisy_petal;

FORCE_INLINE void daisy_petal_init(daisy_petal *p) 
{
	// Init Daisy Seed
	// TODO: decide if this should be a part of the bsp init, 
	//     or if users should have to init seed and board.
	daisy_seed_init(&p->seed);

	// Init Switches
	uint8_t sw_pins[SW_LAST]				= {SW_1_PIN, SW_2_PIN, SW_3_PIN, SW_4_PIN, SW_5_PIN, SW_6_PIN, SW_7_PIN};
	dsy_gpio_port sw_ports[SW_LAST]			= {SW_1_PORT, SW_2_PORT, SW_3_PORT, SW_4_PORT, SW_5_PORT, SW_6_PORT, SW_7_PORT};
	for(uint8_t i = 0; i < SW_LAST; i++) 
	{
		p->switches[i].pin_config.port = sw_ports[i];
		p->switches[i].pin_config.pin  = sw_pins[i];
		p->switches[i].polarity				= DSY_SWITCH_POLARITY_INVERTED;
		p->switches[i].pull					= DSY_SWITCH_PULLUP;
		p->switches[i].type					= DSY_SWITCH_TYPE_TOGGLE;
		dsy_switch_init(&p->switches[i]);
	}

	// Encoder
	// TODO Add Encoder support

	// Init LED Driver
	// 2x PCA9685 addresses 0x00, and 0x01
	uint8_t addr[2] = {0x00, 0x01};
	dsy_led_driver_init(&p->seed.LED_DRIVER_I2C, addr, 2);

	// ADC
	uint8_t channel_order[KNOB_LAST + CV_LAST] = {
		DSY_ADC_PIN_CHN10,
		DSY_ADC_PIN_CHN17,
		DSY_ADC_PIN_CHN15,
		DSY_ADC_PIN_CHN5,
		DSY_ADC_PIN_CHN7,
		DSY_ADC_PIN_CHN3,
		DSY_ADC_PIN_CHN4, // Expression 0-5V
	};
	p->seed.adc_handle.channels = KNOB_LAST + CV_LAST;
	for(uint8_t i = 0; i < 5; i++)
	{
		p->seed.adc_handle.active_channels[i] = channel_order[i];
	}
	dsy_adc_init(&p->seed.adc_handle);
}
	

#ifdef __cplusplus
}
#endif
#endif
