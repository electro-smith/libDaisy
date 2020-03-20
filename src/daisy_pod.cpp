#include "daisy_pod.h"

using namespace daisy;

void DaisyPod::Init()
{
    uint8_t blocksize = 48;

    daisy_seed_init(&seed);
    InitButtons();
    InitEncoder();
    InitLeds();
    InitKnobs();
}

void DaisyPod::StartAudio(dsy_audio_callback cb)
{
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, cb);
    dsy_audio_start(DSY_AUDIO_INTERNAL);
}

void DaisyPod::ChangeAudioCallback(dsy_audio_callback cb)
{
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, cb);
}

void DaisyPod::StartAdc()
{
    dsy_adc_start();
}

void DaisyPod::SetLed(Led ld, float bright)
{
    dsy_gpio_write(&leds[ld], bright);
}

void DaisyPod::ClearLeds()
{
    for(int i = 0; i < LED_LAST; i++)
    {
        SetLed(static_cast<Led>(i), 0);
    }
}

void DaisyPod::UpdateLeds()
{
    dsy_led_driver_update();
}

void DaisyPod::InitButtons()
{
    // button1
    dsy_gpio_pin button1_pin;
    button1_pin.pin  = SW_1_PIN;
    button1_pin.port = SW_1_PORT;
    button1.Init(button1_pin, 1000.0f);

    // button2
    dsy_gpio_pin button2_pin;
    button2_pin.pin  = SW_2_PIN;
    button2_pin.port = SW_2_PORT;
    button2.Init(button2_pin, 1000.0f);
    /* ideal code:
    button2.Init(seed.ADC1, 1000.0f);
    button2.Init(seed.pin(1), 1000.0f); */

    buttons[BUTTON_1] = &button1;
    buttons[BUTTON_2] = &button2;
}

void DaisyPod::InitEncoder()
{
    encoder.Init({ENC_A_PORT, ENC_A_PIN},
                 {ENC_B_PORT, ENC_B_PIN},
                 {ENC_CLICK_PORT, ENC_CLICK_PIN},
                 1000.0f);
}

void DaisyPod::InitLeds()
{
    // LEDs are just going to be on/off for now.
    // TODO: Add PWM support

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
}
void DaisyPod::InitKnobs()
{
    uint8_t channel_order[KNOB_LAST] = {DSY_ADC_PIN_CHN11, DSY_ADC_PIN_CHN10};
    seed.adc_handle.channels         = KNOB_LAST;
    for(uint8_t i = 0; i < KNOB_LAST; i++)
    {
        seed.adc_handle.active_channels[i] = channel_order[i];
    }
    seed.adc_handle.oversampling = DSY_ADC_OVS_32;
    dsy_adc_init(&seed.adc_handle);
    
    knobs[KNOB_1] = &knob1;
    knobs[KNOB_2] = &knob2;
    for(int i = 0; i < KNOB_LAST; i++) {
        knobs[i]->Init(dsy_adc_get_rawptr(i), 1000.0f);
    }
}