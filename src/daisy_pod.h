#pragma once
#ifndef DSY_POD_BSP_H
#define DSY_POD_BSP_H
#include "daisy_seed.h"

#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE
#endif

#define SW_1_PORT seed_ports[29]
#define SW_1_PIN seed_pins[29]
#define SW_2_PORT seed_ports[28]
#define SW_2_PIN seed_pins[28]

#define ENC_A_PORT seed_ports[27]
#define ENC_A_PIN seed_pins[27]
#define ENC_B_PORT seed_ports[26]
#define ENC_B_PIN seed_pins[26]
#define ENC_CLICK_PORT seed_ports[1]
#define ENC_CLICK_PIN seed_pins[1]

#define LED_1_R_PORT seed_ports[21]
#define LED_1_R_PIN seed_pins[21]
#define LED_1_G_PORT seed_ports[20]
#define LED_1_G_PIN seed_pins[20]
#define LED_1_B_PORT seed_ports[19]
#define LED_1_B_PIN seed_pins[19]

#define LED_2_R_PORT seed_ports[0]
#define LED_2_R_PIN seed_pins[0]
#define LED_2_G_PORT seed_ports[25]
#define LED_2_G_PIN seed_pins[25]
#define LED_2_B_PORT seed_ports[24]
#define LED_2_B_PIN seed_pins[24]

using namespace daisy;

enum
{
	SW_1,
	SW_2,
	SW_LAST,
}; enum
{
	KNOB_1,
	KNOB_2,
	KNOB_LAST,
};
enum
{
	CV_LAST,
};
enum
{
	LED_1_R,	
	LED_1_G,	
	LED_1_B,	
	LED_2_R,	
	LED_2_G,	
	LED_2_B,	
	LED_LAST,
};

typedef struct
{
	daisy_handle seed;
	dsy_gpio   leds[LED_LAST];
    Switch switches[SW_LAST];
    Encoder encoder;
	float knobs[KNOB_LAST];
} daisy_pod;

FORCE_INLINE void daisy_pod_init(daisy_pod *p) 
{
	uint8_t sw_pins[SW_LAST]				= {SW_1_PIN, SW_2_PIN };
	dsy_gpio_port sw_ports[SW_LAST]			= {SW_1_PORT, SW_2_PORT};
	dsy_gpio_port led_ports[LED_LAST]			  = {
									 LED_1_R_PORT,
									 LED_1_G_PORT,
									 LED_1_B_PORT,
									 LED_2_R_PORT,
									 LED_2_G_PORT,
									 LED_2_B_PORT};
	uint8_t		  led_pins[LED_LAST]			  = {
							     	 LED_1_R_PIN,
									 LED_1_G_PIN,
									 LED_1_B_PIN,
									 LED_2_R_PIN,
									 LED_2_G_PIN,
									 LED_2_B_PIN};
	daisy_seed_init(&p->seed);
	// Init Switches
	for(uint8_t i = 0; i < SW_LAST; i++) 
	{
        p->switches[i].Init({sw_ports[i], sw_pins[i]}, 1000.0f);
	}

	// LEDs are just going to be on/off for now.
	// TODO: Add PWM support
	for(uint8_t i = 0; i < LED_LAST; i++)
	{
		p->leds[i].pin.port = led_ports[i];
		p->leds[i].pin.pin  = led_pins[i];
		p->leds[i].mode		= DSY_GPIO_MODE_OUTPUT_PP;
		p->leds[i].pull		= DSY_GPIO_NOPULL;
		dsy_gpio_init(&p->leds[i]);
	}

	// Encoder
    p->encoder.Init({ENC_A_PORT, ENC_A_PIN}, 
                    {ENC_B_PORT, ENC_B_PIN},
                    {ENC_CLICK_PORT, ENC_CLICK_PIN},
                    1000.0f);

	uint8_t channel_order[KNOB_LAST] = {DSY_ADC_PIN_CHN11, DSY_ADC_PIN_CHN10};
	p->seed.adc_handle.channels		 = KNOB_LAST;
	for(uint8_t i = 0; i < KNOB_LAST; i++)
	{
		p->seed.adc_handle.active_channels[i] = channel_order[i];
	}
    p->seed.adc_handle.oversampling = DSY_ADC_OVS_32;
	dsy_adc_init(&p->seed.adc_handle);
}

#endif


