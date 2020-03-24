#pragma once
#ifndef DSY_FIELD_BSP_H
#define DSY_FIELD_BSP_H
#include "daisy_seed.h"

#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE
#endif

// Hardware related defines.
#define SW_1_PIN 29
#define SW_2_PIN 28
#define SW_3_PIN 27
#define GATE_OUT_PIN 0
#define GATE_IN_PIN 1

// keyboard switches shift register
#define KB_SW_SR_CS_PIN 8
#define KB_SW_SR_CLK_PIN 9
#define KB_SW_SR_D1_PIN 10
#define KB_SW_SR_D2_PIN 11

// UART for MIDI via TRS jacks on Field
#define MIDI_OUT_PIN 14
#define MIDI_IN_PIN 15

// CD4051 Select Pin controls
#define MUX_SEL_0_PIN 21
#define MUX_SEL_1_PIN 20
#define MUX_SEL_2_PIN 19

#define LED_DRIVER_I2C i2c1_handle

// enums for controls, etc.
enum
{
    SW_2, // tactile switch
    SW_1, // tactile switch
    SW_3, // toggle
    SW_LAST,
};

// All knobs connect to ADC1_INP10 via CD4051 mux
enum
{
    KNOB_1,
    KNOB_3,
    KNOB_5,
    KNOB_2,
    KNOB_4,
    KNOB_6,
    KNOB_7,
    KNOB_8,
    KNOB_LAST,
};

enum
{
    CV_1, // Connected to ADC1_INP17
    CV_2, // Connected  to ADC1_INP15
    CV_3, // Connected to ADC1_INP4
    CV_4, // Connected to ADC1_INP11
    CV_LAST,
};

enum
{
    LED_KEY_A8,
    LED_KEY_A7,
    LED_KEY_A6,
    LED_KEY_A5,
    LED_KEY_A4,
    LED_KEY_A3,
    LED_KEY_A2,
    LED_KEY_A1,
    LED_KEY_B1,
    LED_KEY_B2,
    LED_KEY_B3,
    LED_KEY_B4,
    LED_KEY_B5,
    LED_KEY_B6,
    LED_KEY_B7,
    LED_KEY_B8,
    LED_KNOB_1,
    LED_KNOB_2,
    LED_KNOB_3,
    LED_KNOB_4,
    LED_KNOB_5,
    LED_KNOB_6,
    LED_KNOB_7,
    LED_KNOB_8,
    LED_SW_1,
    LED_SW_2,
    LED_LAST
};

typedef struct
{
    daisy::DaisySeed   seed;
    daisy::Switch      switches[SW_LAST];
    dsy_gpio           gate_in, gate_out;
    dsy_sr_4021_handle keyboard_sr;
    float              knobs[KNOB_LAST];
    float              cvs[CV_LAST];
} daisy_field;

FORCE_INLINE void daisy_field_init(daisy_field *p)
{
    uint8_t sw_pins[SW_LAST] = {SW_1_PIN, SW_2_PIN, SW_3_PIN};
    //	dsy_gpio_port sw_ports[SW_LAST]			= {SW_1_PORT, SW_2_PORT, SW_3_PORT};

    // Init Daisy Seed
    p->seed.Configure();
    p->seed.Init();

    // Init Switches
    for(uint8_t i = 0; i < SW_LAST; i++)
    {
        p->switches[i].Init(p->seed.GetPin(sw_pins[i]), 1000.0f);
    }

    // Init Gate Input
    p->gate_in.pin  = p->seed.GetPin(GATE_IN_PIN);
    p->gate_in.mode = DSY_GPIO_MODE_INPUT;
    p->gate_in.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&p->gate_in);

    // Init Gate Output
    p->gate_out.pin  = p->seed.GetPin(GATE_OUT_PIN);
    p->gate_out.mode = DSY_GPIO_MODE_OUTPUT_PP;
    p->gate_out.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&p->gate_out);

    // Init LED Driver
    // 2x PCA9685 addresses 0x00, and 0x01
    // TODO: add multidriver support
    uint8_t addr[2] = {0x00, 0x02};

    dsy_led_driver_init(&p->seed.LED_DRIVER_I2C, addr, 2);

    // Init Keyboard Switches
    // TODO: add cd4021 with parallel data support
    p->keyboard_sr.pin_config[DSY_SR_4021_PIN_CS]
        = p->seed.GetPin(KB_SW_SR_CS_PIN);
    p->keyboard_sr.pin_config[DSY_SR_4021_PIN_CLK]
        = p->seed.GetPin(KB_SW_SR_CLK_PIN);
    p->keyboard_sr.pin_config[DSY_SR_4021_PIN_DATA]
        = p->seed.GetPin(KB_SW_SR_D1_PIN);
    p->keyboard_sr.pin_config[DSY_SR_4021_PIN_DATA2]
        = p->seed.GetPin(KB_SW_SR_D2_PIN);
    p->keyboard_sr.num_daisychained                       = 1;
    p->keyboard_sr.num_parallel                           = 2;
    dsy_sr_4021_init(&p->keyboard_sr);

    // Init ADC (currently in daisy_seed).
    uint8_t channel_order[5]    = {DSY_ADC_PIN_CHN10,
                                DSY_ADC_PIN_CHN17,
                                DSY_ADC_PIN_CHN15,
                                DSY_ADC_PIN_CHN4,
                                DSY_ADC_PIN_CHN11};
    p->seed.adc_handle.channels = 5;
    p->seed.adc_handle.mux_channels[DSY_ADC_PIN_CHN10]
        = 8; // Use Mux on Channel 0
    p->seed.adc_handle.mux_pin_config[DSY_ADC_PIN_CHN10][MUX_SEL_0]
        = p->seed.GetPin(MUX_SEL_0_PIN);
    p->seed.adc_handle.mux_pin_config[DSY_ADC_PIN_CHN10][MUX_SEL_1]
        = p->seed.GetPin(MUX_SEL_1_PIN);
    p->seed.adc_handle.mux_pin_config[DSY_ADC_PIN_CHN10][MUX_SEL_2]
        = p->seed.GetPin(MUX_SEL_2_PIN);
    for(uint8_t i = 0; i < 5; i++)
    {
        p->seed.adc_handle.active_channels[i] = channel_order[i];
    }
    p->seed.adc_handle.oversampling = DSY_ADC_OVS_32;
    dsy_adc_init(&p->seed.adc_handle);
	// Start timer
    dsy_tim_start();
}

#endif
