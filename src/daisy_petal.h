#pragma once
#ifndef DSY_PETAL_H
#define DSY_PETAL_H

#include "daisy_seed.h"

//typedef struct
//{
//	daisy_handle seed;
//	//dsy_switch switches[SW_LAST];
//    daisy::Switch switches[SW_LAST];
//	float knobs[KNOB_LAST];
//	float cvs[CV_LAST];
//} daisy_petal;

namespace daisy
{
class DaisyPetal
{
  public:
    // enums for controls, etc.
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

    // All knobs connect to ADC1_INP10 via CD4051 mux
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
    // Hardware Accessors
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

    // TODO breakout to individuals with an array of pointers.
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
        return dsy_adc_get_rawptr(chn);
    }

    float  sample_rate_, callback_rate_;
    size_t block_size_;
};

//FORCE_INLINE void daisy_petal_init(daisy_petal* p)
//{
//    // Init Daisy Seed
//    // TODO: decide if this should be a part of the bsp init,
//    //     or if users should have to init seed and board.
//    daisy_seed_init(&p->seed);
//
//    // Init Switches
//    uint8_t sw_pins[SW_LAST] = {
//        SW_1_PIN, SW_2_PIN, SW_3_PIN, SW_4_PIN, SW_5_PIN, SW_6_PIN, SW_7_PIN};
//    dsy_gpio_port sw_ports[SW_LAST] = {SW_1_PORT,
//                                       SW_2_PORT,
//                                       SW_3_PORT,
//                                       SW_4_PORT,
//                                       SW_5_PORT,
//                                       SW_6_PORT,
//                                       SW_7_PORT};
//    for(uint8_t i = 0; i < SW_LAST; i++)
//    {
//        p->switches[i].Init({sw_ports[i], sw_pins[i]},
//                            1000.0f,
//                            daisy::Switch::TYPE_TOGGLE,
//                            daisy::Switch::POLARITY_INVERTED,
//                            daisy::Switch::PULL_UP);
//    }
//
//    // Encoder
//    // TODO Add Encoder support
//
//    // Init LED Driver
//    // 2x PCA9685 addresses 0x00, and 0x01
//    uint8_t addr[2] = {0x00, 0x01};
//    dsy_led_driver_init(&p->seed.LED_DRIVER_I2C, addr, 2);
//
//    // ADC
//    uint8_t channel_order[KNOB_LAST + CV_LAST] = {
//        DSY_ADC_PIN_CHN10,
//        DSY_ADC_PIN_CHN17,
//        DSY_ADC_PIN_CHN15,
//        DSY_ADC_PIN_CHN5,
//        DSY_ADC_PIN_CHN7,
//        DSY_ADC_PIN_CHN3,
//        DSY_ADC_PIN_CHN4, // Expression 0-5V
//    };
//    p->seed.adc_handle.channels = KNOB_LAST + CV_LAST;
//    for(uint8_t i = 0; i < 5; i++)
//    {
//        p->seed.adc_handle.active_channels[i] = channel_order[i];
//    }
//    dsy_adc_init(&p->seed.adc_handle);
//}

} // namespace daisy

#endif
