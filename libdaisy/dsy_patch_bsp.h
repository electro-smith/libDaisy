#pragma once
#ifndef DSY_PATCH_BSP_H
#define DSY_PATCH_BSP_H

// These are defines/macros, etc. mapped to:
// Daisy Patch Rev2 w/ Daisy Seed rev2
// The pins for the Buttons are different for Rev1
// Otherwsie these should be valid for both revisions so far.

#define BUTTON_1_PORT GPIOA
#define BUTTON_1_PIN GPIO_PIN_8
#define BUTTON_2_PORT GPIOB
#define BUTTON_2_PIN GPIO_PIN_12
#define TOGGLE_PORT GPIOG
#define TOGGLE_PIN GPIO_PIN_11
#define GATE_1_PORT GPIOB
#define GATE_1_PIN GPIO_PIN_5
#define GATE_2_PORT GPIOB
#define GATE_2_PIN GPIO_PIN_4
#define GATE_OUT_PORT GPIOG
#define GATE_OUT_PIN GPIO_PIN_10
// We'll test DAC in a sec, first I'm just going to check as GPIO
#define DAC_OUT_1_PORT GPIOA
#define DAC_OUT_1_PIN GPIO_PIN_5
#define DAC_OUT_2_PORT GPIOA
#define DAC_OUT_2_PIN GPIO_PIN_4

// This is part of Seed, not patch.
// So it should likely just become embedded into sys_init 
#define ONBOARD_LED_PORT GPIOC
#define ONBOARD_LED_PIN GPIO_PIN_7

// Order of ADC Channels for accessing dsy_adc.h
enum
{
	DSY_PATCH_KNOB_1,
	DSY_PATCH_CV_1,
	DSY_PATCH_KNOB_2,
	DSY_PATCH_KNOB_3,
	DSY_PATCH_KNOB_4,
	DSY_PATCH_CV_2,
	DSY_PATCH_CV_3,
	DSY_PATCH_CV_4,
	DSY_PATCH_CV_LAST
};

#endif
