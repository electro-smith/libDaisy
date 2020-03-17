// # Daisy Pod BSP
// ## Description
// Class that handles initializing all of the hardware specific to the Daisy Patch Board.
//
// Helper funtions are also in place to provide easy access to built-in controls and peripherals.
//
// ## Credits
// **Author:** Stephen Hensley
// **Date Added:** November 2019

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

#ifdef __cplusplus
namespace daisy
{
class daisy_pod
{
  public:
    enum sw
    {
        BUTTON_1,
        BUTTON_2,
        BUTTON_LAST,
    };
    enum knob
    {
        KNOB_1,
        KNOB_2,
        KNOB_LAST,
    };

    enum led
    {
        LED_1_R,
        LED_1_G,
        LED_1_B,
        LED_2_R,
        LED_2_G,
        LED_2_B,
        LED_LAST,
    };

    daisy_pod() {}
    ~daisy_pod() {}

    void Init();

    inline void StartAudio(dsy_audio_callback cb)
    {
        dsy_audio_set_callback(DSY_AUDIO_INTERNAL, cb);
        dsy_audio_start(DSY_AUDIO_INTERNAL);
    }

    inline void ChangeAudioCallback(dsy_audio_callback cb)
    {
        dsy_audio_set_callback(DSY_AUDIO_INTERNAL, cb);
    }

    inline void StartAdc() { dsy_adc_start(); }

    inline void SetLed(led ld, float bright)
    {
        dsy_led_driver_set_led(ld, bright);
    }

    inline void ClearLeds()
    {
        for(int i = 0; i < LED_LAST; i++)
        {
            SetLed(static_cast<led>(i), 0);
        }
    }

    inline void UpdateLeds() { dsy_led_driver_update(); }

    inline AnalogControl GetCtrl(ctrl c) { return pctrl[c]; }

    daisy_handle  seed;
    Encoder       encoder;
    AnalogControl knob1, knob2;
    Switch        button1, button2;
    dsy_gpio      led1_r, led1_g, led1_b;
    dsy_gpio      led2_r, led2_g, led2_b;

    dsy_gpio leds[LED_LAST];

  private:
    inline uint16_t* adc_ptr(const uint8_t chn)
    {
        return dsy_adc_get_rawptr(chn);
    }
    AnalogControl pctrl[CV_LAST];
};

} // namespace daisy
