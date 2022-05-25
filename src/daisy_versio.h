#pragma once
#include "daisy_seed.h"

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
        LED_LAST
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
        KNOB_LAST
    };

    enum AV_TOGGLE3
    {
        SW_0,
        SW_1,
        SW_LAST
    };

    DaisyVersio() {}
    ~DaisyVersio() {}

    /**Initializes the Versio, and all of its hardware.*/
    void Init(bool boost = false);

    /** 
    Wait some ms before going on.
    \param del Delay time in ms.
    */
    void DelayMs(size_t del);

    /** Starts the callback
    \param cb Interleaved callback function
    */
    void StartAudio(AudioHandle::InterleavingAudioCallback cb);

    /** Starts the callback
    \param cb Non-interleaved callback function
    */
    void StartAudio(AudioHandle::AudioCallback cb);

    /**
       Switch callback functions
       \param cb New interleaved callback function.
    */
    void ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb);

    /**
       Switch callback functions
       \param cb New non-interleaved callback function.
    */
    void ChangeAudioCallback(AudioHandle::AudioCallback cb);

    /** Stops the audio if it is running. */
    void StopAudio();

    /** Sets the number of samples processed per channel by the audio callback.
     */
    void SetAudioBlockSize(size_t size);

    /** Returns the number of samples per channel in a block of audio. */
    size_t AudioBlockSize();

    /** Updates the Audio Sample Rate, and reinitializes.
     ** Audio must be stopped for this to work.
     */
    void SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate);

    /** Returns the audio sample rate in Hz as a floating point number.
     */
    float AudioSampleRate();

    /** Returns the rate in Hz that the Audio callback is called */
    float AudioCallbackRate();

    /** Start analog to digital conversion.*/
    void StartAdc();

    /** Stop converting ADCs */
    void StopAdc();

    /** Normalize ADC CV input. Call this once per main loop update to normalize CV input to range (0.0f, 1.0f) */
    void ProcessAnalogControls();

    /** Does what it says */
    inline void ProcessAllControls() { ProcessAnalogControls(); }

    /** Returns true if momentary switch is pressed */
    bool SwitchPressed();

    /** Returns true if gate in is HIGH */
    bool Gate();

    /** Set an LED (idx < 4) to a color */
    void SetLed(size_t idx, float red, float green, float blue);

    /** Get Knob Value, float from 0.0f to 1.0f */
    float GetKnobValue(int idx);

    /** Update LED PWM state. Call this once per main loop update to correctly display led colors */
    void UpdateLeds();

    /* Call this once per main loop update to test the Versio Hardware
     * Each input (1 gate in, 1 momentary in, 2 3-position switches, 7 knobs) should change some or all of the output LEDs colors
     **/
    void UpdateExample();

    DaisySeed     seed;
    RgbLed        leds[LED_LAST];
    AnalogControl knobs[KNOB_LAST];
    Switch        tap;
    GateIn        gate;
    Switch3       sw[SW_LAST];

  private:
    void SetHidUpdateRates();
};

} // namespace daisy
