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

namespace daisy
{
class DaisyPod
{
  public:
    enum Sw
    {
        BUTTON_1,
        BUTTON_2,
        BUTTON_LAST,
    };

    enum Knob
    {
        KNOB_1,
        KNOB_2,
        KNOB_LAST,
    };

    enum Led
    {
        LED_1_R,
        LED_1_G,
        LED_1_B,
        LED_2_R,
        LED_2_G,
        LED_2_B,
        LED_LAST,
        t
    };

    DaisyPod() {}
    ~DaisyPod() {}

    // Functions
    // Init related stuff.
    void Init();
    void DelayMs(size_t del);
    // Audio Block size defaults to 48.
    // Change it using this function before StartingAudio
    void SetAudioBlockSize(size_t size);
    void StartAudio(dsy_audio_callback cb);
    void ChangeAudioCallback(dsy_audio_callback cb);
    void StartAdc();
    // Hardware Accessors
    float  AudioSampleRate();
    size_t AudioBlockSize();
    float  AudioCallbackRate();
    void UpdateKnobs();
    float  GetKnobValue(Knob k);
    void   DebounceControls();
    void   SetLed(Led ld, float bright);
    void   ClearLeds();
    void   UpdateLeds();
    // Public Members.
    daisy_handle  seed;
    Encoder       encoder;
    AnalogControl knob1, knob2, *knobs[KNOB_LAST];
    Switch        button1, button2, *buttons[BUTTON_LAST];
    dsy_gpio      led1_r, led1_g, led1_b;
    dsy_gpio      led2_r, led2_g, led2_b, leds[LED_LAST];

  private:
    void             InitButtons();
    void             InitEncoder();
    void             InitLeds();
    void             InitKnobs();
    inline uint16_t* adc_ptr(const uint8_t chn)
    {
        return dsy_adc_get_rawptr(chn);
    }
    float  sample_rate_, callback_rate_;
    size_t block_size_;
};

} // namespace daisy
#endif
