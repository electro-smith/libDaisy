#pragma once
#ifndef DSY_PATCH_BSP_H
#define DSY_PATCH_BSP_H

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

#endif
