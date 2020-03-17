#include "daisy_pod.h"

using namespace daisy;

void daisy_pod::Init()
{
    blocksize = 48;

    // init seed
    daisy_seed_init(&seed);

    // init button1
    dsy_gpio_pin button1_pin;
    button1_pin.pin  = SW_1_PIN;
    button1_pin.port = SW_1_PORT;
    button1.Init(button1_pin, 1000.0f);

    // init button2
    dsy_gpio_pin button2_pin;
    button2_pin.pin  = SW_2_PIN;
    button2_pin.port = SW_2_PORT;
    button2.Init(button2_pin, 1000.0f);

    // init encoder
    encoder.Init({ENC_A_PORT, ENC_A_PIN},
                 {ENC_B_PORT, ENC_B_PIN},
                 {ENC_CLICK_PORT, ENC_CLICK_PIN},
                 1000.0f);

    // LEDs are just going to be on/off for now.
    // TODO: Add PWM support
    // init leds
    dsy_gpio_port led_ports[LED_LAST] = {LED_1_R_PORT,
                                         LED_1_G_PORT,
                                         LED_1_B_PORT,
                                         LED_2_R_PORT,
                                         LED_2_G_PORT,
                                         LED_2_B_PORT};
    uint8_t       led_pins[LED_LAST]  = {LED_1_R_PIN,
                                  LED_1_G_PIN,
                                  LED_1_B_PIN,
                                  LED_2_R_PIN,
                                  LED_2_G_PIN,
                                  LED_2_B_PIN};

    for(uint8_t i = 0; i < LED_LAST; i++)
    {
        leds[i].pin.port = led_ports[i];
        leds[i].pin.pin  = led_pins[i];
        leds[i].mode     = DSY_GPIO_MODE_OUTPUT_PP;
        leds[i].pull     = DSY_GPIO_NOPULL;
        dsy_gpio_init(&leds[i]);
    }

    uint8_t channel_order[KNOB_LAST] = {DSY_ADC_PIN_CHN11, DSY_ADC_PIN_CHN10};
    p->seed.adc_handle.channels      = KNOB_LAST;
    for(uint8_t i = 0; i < KNOB_LAST; i++)
    {
        p->seed.adc_handle.active_channels[i] = channel_order[i];
    }
    p->seed.adc_handle.oversampling = DSY_ADC_OVS_32;
    dsy_adc_init(&p->seed.adc_handle);
}