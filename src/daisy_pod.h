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
    void UpdateAnalogControls();
    float  GetKnobValue(Knob k);
    void   DebounceControls();
    void   ClearLeds();
    void   UpdateLeds();
    // Public Members.
    DaisySeed seed;
    Encoder       encoder;
    AnalogControl knob1, knob2, *knobs[KNOB_LAST];
    Switch        button1, button2, *buttons[BUTTON_LAST];
    RgbLed        led1, led2;

  private:
    void             InitButtons();
    void             InitEncoder();
    void             InitLeds();
    void             InitKnobs();
    float  sample_rate_, callback_rate_;
    size_t block_size_;
};

} // namespace daisy
#endif
