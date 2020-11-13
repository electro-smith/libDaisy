#pragma once
#include "daisy_seed.h"
#include "hid/switch3.h"

namespace daisy
{
/**
    @brief Class that handles initializing all of the hardware specific to the Desmodus Versio hardware. \n 
    Helper funtions are also in place to provide easy access to built-in controls and peripherals.
    @author Ankoor Apte, Noise Engineering
    @date October 2020
    @ingroup boards
*/
class DaisyVersio
{
  public:
    
    // ENUMS to help index 4 LEDs, 7 knobs/CV inputs, 2 3-position switches
    enum AV_LEDS
    {
        LED_0,
        LED_1,
        LED_2,
        LED_3,
        LED_COUNT
    };

    enum AV_KNOBS
    {
        KNOB_0,
        KNOB_1,
        KNOB_2,
        KNOB_3,
        KNOB_4,
        KNOB_5,
        KNOB_6,
        KNOB_COUNT
    };

    enum AV_TOGGLE3
    {
        SW_0,
        SW_1,
        SW_COUNT
    };

    DaisyVersio() {}
    ~DaisyVersio() {}
    
    /**Initializes the Versio, and all of its hardware.*/
    void Init();
    
    /** 
    Wait some ms before going on.
    \param del Delay time in ms.
    */
    void DelayMs(size_t del);

    /** Starts the callback
    \cb Interleaved callback function
    */
    void StartAudio(AudioHandle::InterleavingAudioCallback cb);

    /** Starts the callback
    \cb Non-interleaved callback function
    */
    void StartAudio(AudioHandle::AudioCallback cb);
    
    /** Start analog to digital conversion.*/
    void StartAdc() { seed.adc.Start(); }

    float AudioSampleRate() { return seed.AudioSampleRate(); }

    /** Normalize ADC CV input. Call this once per main loop update to normalize CV input to range (0.0f, 1.0f) */
    void ProcessAdc();
    
    /** Returns true if momentary switch is pressed */
    bool SwitchPressed() { return tap_.Pressed(); }
    
    /** Returns true if gate in is HIGH */
    bool Gate() { return !gate_.State(); }

    /** Set an LED (idx < 4) to a color */
    void SetLed(size_t idx, float red, float green, float blue)
    {
        leds[idx].Set(red, green, blue);
    }

    /** Get Knob Value, float from 0.0f to 1.0f */
    float GetKnobValue(int idx);
    
    /** Update LED PWM state. Call this once per main loop update to correctly display led colors */
    void UpdateLeds();

    /* Call this once per main loop update to test the Versio Hardware
     * Each input (1 gate in, 1 momentary in, 2 3-position switches, 7 knobs) should change some or all of the output LEDs colors
     **/
    void UpdateExample();

    DaisySeed seed;
    RgbLed    leds[LED_COUNT];

  private:
    float         samplerate_, blockrate_;
    size_t        blocksize_;
    AnalogControl knobs_[KNOB_COUNT];
    Switch        tap_;
    GateIn        gate_;

    Switch3 sw_[SW_COUNT];
};

} // namespace daisy
