#include "daisy_patch.h"

using namespace daisy;

void daisy_patch::Init() 
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
	uint8_t channel_order[8] = {DSY_ADC_PIN_CHN3,
								DSY_ADC_PIN_CHN10,
								DSY_ADC_PIN_CHN7,
								DSY_ADC_PIN_CHN11,
								DSY_ADC_PIN_CHN4,
								DSY_ADC_PIN_CHN5,
								DSY_ADC_PIN_CHN15,
								DSY_ADC_PIN_CHN17};
	seed.adc_handle.channels
		= 8; // only initializing 8 primary channels.
	for(uint8_t i = 0; i < 8; i++)
	{
		seed.adc_handle.active_channels[i] = channel_order[i];
	}
	seed.adc_handle.oversampling = DSY_ADC_OVS_32;
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
	cv1.init_bipolar_cv(adc_ptr(CV_1), SAMPLE_RATE);
	cv2.init_bipolar_cv(adc_ptr(CV_2), SAMPLE_RATE);
	cv3.init_bipolar_cv(adc_ptr(CV_3), SAMPLE_RATE);
	cv4.init_bipolar_cv(adc_ptr(CV_4), SAMPLE_RATE);

	// LEDs
	uint8_t addr = 0x00;
	dsy_led_driver_init(&seed.LED_DRIVER_I2C, &addr, 1);
}
