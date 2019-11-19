#pragma once
#ifndef DSY_PATCH_BSP_H
#define DSY_PATCH_BSP_H
#include "dsy_seed.h"

// These are defines/macros, etc. mapped to:
// Daisy Patch Rev2 w/ Daisy Seed rev2
// The pins for the Buttons are different for Rev1
// Otherwsie these should be valid for both revisions so far.

//#define BUTTON_1_PORT GPIOA
//#define BUTTON_1_PIN GPIO_PIN_8
//#define BUTTON_2_PORT GPIOB
//#define BUTTON_2_PIN GPIO_PIN_12
//#define TOGGLE_PORT GPIOG
//#define TOGGLE_PIN GPIO_PIN_11
//#define GATE_1_PORT GPIOB
//#define GATE_1_PIN GPIO_PIN_5
//#define GATE_2_PORT GPIOB
//#define GATE_2_PIN GPIO_PIN_4
//#define GATE_OUT_PORT GPIOG
//#define GATE_OUT_PIN GPIO_PIN_10
//// We'll test DAC in a sec, first I'm just going to check as GPIO
//#define DAC_OUT_1_PORT GPIOA
//#define DAC_OUT_1_PIN GPIO_PIN_5
//#define DAC_OUT_2_PORT GPIOA
//#define DAC_OUT_2_PIN GPIO_PIN_4

// This is part of Seed, not patch.
// So it should likely just become embedded into sys_init 
//#define ONBOARD_LED_PORT GPIOC
//#define ONBOARD_LED_PIN GPIO_PIN_7

#define BUTTON_1_PORT DSY_GPIOA
#define BUTTON_1_PIN 8
#define BUTTON_2_PORT DSY_GPIOB
#define BUTTON_2_PIN 12 
#define TOGGLE_PORT DSY_GPIOG
#define TOGGLE_PIN 11 
#define GATE_1_PORT DSY_GPIOB
#define GATE_1_PIN 5
#define GATE_2_PORT DSY_GPIOB
#define GATE_2_PIN 4
#define GATE_OUT_PORT DSY_GPIOG
#define GATE_OUT_PIN 10



// Order of ADC Channels for accessing dsy_adc.h
enum
{
	DSY_PATCH_KNOB_1,
	DSY_PATCH_KNOB_2,
	DSY_PATCH_KNOB_3,
	DSY_PATCH_KNOB_4,
	DSY_PATCH_CV_1,
	DSY_PATCH_CV_2,
	DSY_PATCH_CV_3,
	DSY_PATCH_CV_4,
	DSY_PATCH_CV_LAST
};

// Mapping of LEDs via dsy_leddriver.h
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


typedef struct
{
	dsy_switch_t button1, button2, toggle;
	dsy_gpio_t   gate_in1, gate_in2, gate_out;
	daisy_handle seed;
} daisy_patch;

FORCE_INLINE void daisy_patch_init(daisy_patch *p) 
{

	p->button1.pin_config.port = BUTTON_1_PORT;
	p->button1.pin_config.pin = BUTTON_1_PIN;
	p->button1.pull = DSY_SWITCH_PULLUP;
	p->button1.polarity		   = DSY_SWITCH_POLARITY_INVERTED;
	p->button1.type			   = DSY_SWITCH_TYPE_MOMENTARY;

	p->button2.pin_config.port = BUTTON_2_PORT;
	p->button2.pin_config.pin = BUTTON_2_PIN;
	p->button2.pull = DSY_SWITCH_PULLUP;
	p->button2.polarity		   = DSY_SWITCH_POLARITY_INVERTED;
	p->button2.type			   = DSY_SWITCH_TYPE_MOMENTARY;

	p->toggle.pin_config.port = TOGGLE_PORT;
	p->toggle.pin_config.pin = TOGGLE_PIN;
	p->toggle.pull = DSY_SWITCH_PULLUP;
	p->toggle.type			   = DSY_SWITCH_TYPE_TOGGLE;

	p->gate_in1.pin.port = GATE_1_PORT;
	p->gate_in1.pin.pin = GATE_1_PIN;
	p->gate_in1.mode	 = DSY_GPIO_MODE_INPUT;

	p->gate_in2.pin.port = GATE_2_PORT;
	p->gate_in2.pin.pin = GATE_2_PIN;
	p->gate_in2.mode	 = DSY_GPIO_MODE_INPUT;

	p->gate_out.pin.port = GATE_OUT_PORT;
	p->gate_out.pin.pin = GATE_OUT_PIN;
	p->gate_out.pull	 = DSY_GPIO_NOPULL;
	p->gate_out.mode	 = DSY_GPIO_MODE_OUTPUT_PP;

	dsy_switch_init(&p->button1);
	dsy_switch_init(&p->button2);
	dsy_switch_init(&p->toggle);
	dsy_gpio_init(&p->gate_in1);
	dsy_gpio_init(&p->gate_in2);
	dsy_gpio_init(&p->gate_out);
	// ADC related
	uint8_t channel_order[8]		  = {DSY_ADC_PIN_CHN3,
								 DSY_ADC_PIN_CHN10,
								 DSY_ADC_PIN_CHN7,
								 DSY_ADC_PIN_CHN11,
								 DSY_ADC_PIN_CHN4,
								 DSY_ADC_PIN_CHN5,
								 DSY_ADC_PIN_CHN15,
								 DSY_ADC_PIN_CHN17};
	p->seed.adc_handle.channels
		= 8; // only initializing 8 primary channels.
	for(uint8_t i = 0; i < 8; i++)
	{
		p->seed.adc_handle.active_channels[i] = channel_order[i];
	}
	dsy_adc_init(&p->seed.adc_handle);
	dsy_dac_init(&p->seed.dac_handle, DSY_DAC_CHN_BOTH);
}

#endif
