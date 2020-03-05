#include "daisy_patch.h"

using namespace daisy;

void daisy_patch::Init() 
{
    size_t blocksize;
    // Audio Blocksize 48 by default.
    blocksize = 48; // (only used for CV inits here).
	// Initialize Hardware
    daisy_seed_init(&seed);
	// Pin config for everything
    gate_in1.pin       = {GATE_1_PORT, GATE_1_PIN};
    gate_in2.pin       = {GATE_2_PORT, GATE_2_PIN};
    gate_out.pin	   = {GATE_OUT_PORT, GATE_OUT_PIN};

    // Switches
    button1.Init({BUTTON_1_PORT, BUTTON_1_PIN}, 1000.0f);
    button2.Init({BUTTON_2_PORT, BUTTON_2_PIN}, 1000.0f);
    toggle.Init({TOGGLE_PORT, TOGGLE_PIN},
                1000.0f,
                Switch::TYPE_TOGGLE,
                Switch::POLARITY_INVERTED,
                Switch::PULL_UP);

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
    seed.adc_handle.channels = 8; // only initializing 8 primary channels.
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
		pctrl[i].Init(adc_ptr(i), SAMPLE_RATE/blocksize);
	}
	for(uint8_t i = CV_1; i < CV_LAST; i++)
	{
		pctrl[i].InitBipolarCv(adc_ptr(i), SAMPLE_RATE/blocksize);
	}
	knob1.Init(adc_ptr(KNOB_1), SAMPLE_RATE/blocksize);
	knob2.Init(adc_ptr(KNOB_2), SAMPLE_RATE/blocksize);
	knob3.Init(adc_ptr(KNOB_3), SAMPLE_RATE/blocksize);
	knob4.Init(adc_ptr(KNOB_4), SAMPLE_RATE/blocksize);
	cv1.InitBipolarCv(adc_ptr(CV_1), SAMPLE_RATE/blocksize);
	cv2.InitBipolarCv(adc_ptr(CV_2), SAMPLE_RATE/blocksize);
	cv3.InitBipolarCv(adc_ptr(CV_3), SAMPLE_RATE/blocksize);
	cv4.InitBipolarCv(adc_ptr(CV_4), SAMPLE_RATE/blocksize);

	// LEDs
	uint8_t addr = 0x00;
	dsy_led_driver_init(&seed.LED_DRIVER_I2C, &addr, 1);
}
