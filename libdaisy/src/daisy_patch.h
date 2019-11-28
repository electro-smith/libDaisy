#pragma once
#ifndef DSY_PATCH_BSP_H
#define DSY_PATCH_BSP_H
#include "daisy_seed.h"

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

#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE
#endif

#define LED_DRIVER_I2C i2c1_handle

// Leaving non-cplusplus version in place below so as not to break examples yet...
#ifdef __cplusplus
namespace daisy
{
	enum
	{
		KNOB_1,
		KNOB_2,
		KNOB_3,
		KNOB_4,
		CV_1,
		CV_2,
		CV_3,
		CV_4,
		CV_LAST,
		KNOB_LAST = CV_1,
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
	class daisy_patch
	{
	  public:
		daisy_patch() {}
		~daisy_patch() {}

		inline void init()
		{
			daisy_seed_init(&seed);
			// Pin config for everything
			button1.pin_config = {BUTTON_1_PORT, BUTTON_1_PIN};
			button2.pin_config = {BUTTON_2_PORT, BUTTON_2_PIN};
			toggle.pin_config  = {TOGGLE_PORT, TOGGLE_PIN};
			gate_in1.pin	   = {GATE_1_PORT, GATE_1_PIN};
			gate_in2.pin	   = {GATE_2_PORT, GATE_2_PIN};
			gate_out.pin	   = {GATE_OUT_PORT, GATE_OUT_PIN};

			// Switches
			button1.pull	 = DSY_SWITCH_PULLUP;
			button1.polarity = DSY_SWITCH_POLARITY_INVERTED;
			button1.type	 = DSY_SWITCH_TYPE_MOMENTARY;
			dsy_switch_init(&button1);

			button2.pull	 = DSY_SWITCH_PULLUP;
			button2.polarity = DSY_SWITCH_POLARITY_INVERTED;
			button2.type	 = DSY_SWITCH_TYPE_MOMENTARY;
			dsy_switch_init(&button2);

			toggle.pull = DSY_SWITCH_PULLUP;
			toggle.type = DSY_SWITCH_TYPE_TOGGLE;
			dsy_switch_init(&toggle);

			// GPIO
			gate_in1.mode = DSY_GPIO_MODE_INPUT;
			dsy_gpio_init(&gate_in1);
			gate_in2.mode = DSY_GPIO_MODE_INPUT;
			dsy_gpio_init(&gate_in2);
			gate_out.mode = DSY_GPIO_MODE_OUTPUT_PP;
			dsy_gpio_init(&gate_out);

			// ADCs
			uint8_t channel_order[8]	= {DSY_ADC_PIN_CHN3,
										   DSY_ADC_PIN_CHN10,
										   DSY_ADC_PIN_CHN7,
										   DSY_ADC_PIN_CHN11,
										   DSY_ADC_PIN_CHN4,
										   DSY_ADC_PIN_CHN5,
										   DSY_ADC_PIN_CHN15,
										   DSY_ADC_PIN_CHN17};
			seed.adc_handle.channels = 8; // only initializing 8 primary channels.
			for(uint8_t i = 0; i < 8; i++)
			{
				seed.adc_handle.active_channels[i] = channel_order[i];
			}
			dsy_adc_init(&seed.adc_handle);
			dsy_dac_init(&seed.dac_handle, DSY_DAC_CHN_BOTH);

			// Higher level hid_ctrls
			for(uint8_t i = 0; i < KNOB_LAST; i++) 
			{
				pctrl[i].init(adc_ptr(i), SAMPLE_RATE);
			}
			for(uint8_t i = CV_1; i < CV_LAST; i++) 
			{
				pctrl[i].init_bipolar_cv(adc_ptr(i), SAMPLE_RATE);
			}
			knob1.init(adc_ptr(KNOB_1), SAMPLE_RATE);
			knob2.init(adc_ptr(KNOB_2), SAMPLE_RATE);
			knob3.init(adc_ptr(KNOB_3), SAMPLE_RATE);
			knob4.init(adc_ptr(KNOB_4), SAMPLE_RATE);

			// LEDs
			uint8_t addr = 0x00;
			dsy_led_driver_init(&seed.LED_DRIVER_I2C, &addr, 1);
		}

		inline hid_ctrl ctrl(uint8_t idx) { return pctrl[idx < CV_LAST ? idx : 0]; }

		daisy_handle seed;
		dsy_switch   button1, button2, toggle;
		dsy_gpio	 gate_in1, gate_in2, gate_out;

		// alternate hid_form with public access
		hid_ctrl knob1, knob2, knob3, knob4;
		hid_ctrl cv1, cv2, cv3, cv4;

	  private:
		inline uint16_t* adc_ptr(const uint8_t chn)
		{
			return dsy_adc_get_rawptr(chn);
		}
		hid_ctrl pctrl[CV_LAST]; 
	};
} // namespace daisy

#else
// Order of ADC Channels for accessing dsy_adc.h
enum
{
	KNOB_1,
	KNOB_2,
	KNOB_3,
	KNOB_4,
	CV_1,
	CV_2,
	CV_3,
	CV_4,
	CV_LAST
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
	dsy_switch   button1, button2, toggle;
	dsy_gpio	 gate_in1, gate_in2, gate_out;
	daisy_handle seed;
} daisy_patch;

FORCE_INLINE void daisy_patch_init(daisy_patch *p)
{
	daisy_seed_init(&p->seed);
	p->button1.pin_config.port = BUTTON_1_PORT;
	p->button1.pin_config.pin  = BUTTON_1_PIN;
	p->button1.pull			   = DSY_SWITCH_PULLUP;
	p->button1.polarity		   = DSY_SWITCH_POLARITY_INVERTED;
	p->button1.type			   = DSY_SWITCH_TYPE_MOMENTARY;

	p->button2.pin_config.port = BUTTON_2_PORT;
	p->button2.pin_config.pin  = BUTTON_2_PIN;
	p->button2.pull			   = DSY_SWITCH_PULLUP;
	p->button2.polarity		   = DSY_SWITCH_POLARITY_INVERTED;
	p->button2.type			   = DSY_SWITCH_TYPE_MOMENTARY;

	p->toggle.pin_config.port = TOGGLE_PORT;
	p->toggle.pin_config.pin  = TOGGLE_PIN;
	p->toggle.pull			  = DSY_SWITCH_PULLUP;
	p->toggle.type			  = DSY_SWITCH_TYPE_TOGGLE;

	p->gate_in1.pin.port = GATE_1_PORT;
	p->gate_in1.pin.pin  = GATE_1_PIN;
	p->gate_in1.mode	 = DSY_GPIO_MODE_INPUT;

	p->gate_in2.pin.port = GATE_2_PORT;
	p->gate_in2.pin.pin  = GATE_2_PIN;
	p->gate_in2.mode	 = DSY_GPIO_MODE_INPUT;

	p->gate_out.pin.port = GATE_OUT_PORT;
	p->gate_out.pin.pin  = GATE_OUT_PIN;
	p->gate_out.pull	 = DSY_GPIO_NOPULL;
	p->gate_out.mode	 = DSY_GPIO_MODE_OUTPUT_PP;

	dsy_switch_init(&p->button1);
	dsy_switch_init(&p->button2);
	dsy_switch_init(&p->toggle);
	dsy_gpio_init(&p->gate_in1);
	dsy_gpio_init(&p->gate_in2);
	dsy_gpio_init(&p->gate_out);
	// ADC related
	uint8_t channel_order[8]	= {DSY_ADC_PIN_CHN3,
								   DSY_ADC_PIN_CHN10,
								   DSY_ADC_PIN_CHN7,
								   DSY_ADC_PIN_CHN11,
								   DSY_ADC_PIN_CHN4,
								   DSY_ADC_PIN_CHN5,
								   DSY_ADC_PIN_CHN15,
								   DSY_ADC_PIN_CHN17};
	p->seed.adc_handle.channels = 8; // only initializing 8 primary channels.
	for(uint8_t i = 0; i < 8; i++)
	{
		p->seed.adc_handle.active_channels[i] = channel_order[i];
	}
	dsy_adc_init(&p->seed.adc_handle);
	dsy_dac_init(&p->seed.dac_handle, DSY_DAC_CHN_BOTH);
}
#endif

#endif
