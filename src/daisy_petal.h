//
#pragma once
#ifndef DSY_PETAL_H
#define DSY_PETAL_H

#include "daisy_seed.h"

namespace daisy
{
class DaisyPetal
{
  public:
    enum Sw
    {
        SW_1, // Footswitch
        SW_2, // Footswitch
        SW_3, // Footswitch
        SW_4, // Footswitch
        SW_5, // Toggle
        SW_6, // Toggle
        SW_7, // Toggle
        SW_LAST,
    };

    enum Knob
    {
        KNOB_1,
        KNOB_2,
        KNOB_3,
        KNOB_4,
        KNOB_5,
        KNOB_6,
        KNOB_LAST,
    };

    enum RingLed
    {
        RING_LED_1,
        RING_LED_2,
        RING_LED_3,
        RING_LED_4,
        RING_LED_5,
        RING_LED_6,
        RING_LED_7,
        RING_LED_8,
        RING_LED_LAST
    };

    enum FootswitchLed
    {
        FOOTSWITCH_LED_1,
        FOOTSWITCH_LED_2,
        FOOTSWITCH_LED_3,
        FOOTSWITCH_LED_4,
        FOOTSWITCH_LED_LAST,
    };

    DaisyPetal() {}
    ~DaisyPetal() {}

    void Init();
    void DelayMs(size_t del);

    void SetAudioBlockSize(size_t size);
    void StartAudio(dsy_audio_callback cb);
    void ChangeAudioCallback(dsy_audio_callback cb);
    void StartAdc();
    float  AudioSampleRate();
    size_t AudioBlockSize();
    float  AudioCallbackRate();
    void   UpdateAnalogControls();
    float  GetKnobValue(Knob k);
    float  GetExpression();
    void   DebounceControls();
    void   ClearLeds();
    void   UpdateLeds();
    void   SetRingLed(RingLed idx, float r, float g, float b);
    void   SetFootswitchLed(FootswitchLed idx, float bright);

    DaisySeed seed;
    Encoder   encoder;

    AnalogControl knob[KNOB_LAST];
    AnalogControl expression;
    Switch        switches[SW_LAST];

    RgbLed ring_led[8];
    Led    footswitch_led[4];

  private:
    void InitSwitches();
    void InitEncoder();
    void InitLeds();
    void InitAnalogControls();

    inline uint16_t* adc_ptr(const uint8_t chn)
    {
        return seed.adc.GetPtr(chn);
    }

    float  sample_rate_, callback_rate_;
    size_t block_size_;
};

} // namespace daisy

#endif
