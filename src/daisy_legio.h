#pragma once
#include "daisy_seed.h"

namespace daisy
{
/**
    @brief Class that handles initializing all of the hardware specific to the Virt Iter Legio hardware. \n 
    Helper funtions are also in place to provide easy access to built-in controls and peripherals.
    @author Eris Fairbanks, Olivia Artz Modular + Noise Engineering
    @date June 2022
    @ingroup boards
*/
class DaisyLegio
{
  public:
    // ENUMS to help index 2 LEDs, 2 controls/CV inputs, 2 3-position switches
    enum LEGIO_LEDS
    {
        LED_LEFT,
        LED_RIGHT,
        LED_LAST
    };

    enum LEGIO_CONTROLS
    {
        CONTROL_PITCH,
        CONTROL_KNOB_TOP,
        CONTROL_KNOB_BOTTOM,
        CONTROL_LAST
    };

    enum LEGIO_TOGGLE3
    {
        SW_LEFT,
        SW_RIGHT,
        SW_LAST
    };

    DaisyLegio() {}
    ~DaisyLegio() {}

    /**Initializes the Legio, and all of its hardware.*/
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

    /** Process digital controls */
    void ProcessDigitalControls();

    /** Normalize ADC CV input. Call this once per main loop update to normalize CV input to range (0.0f, 1.0f) */
    void ProcessAnalogControls();

    /** Does what it says */
    inline void ProcessAllControls()
    {
        ProcessDigitalControls();
        ProcessAnalogControls();
    }

    /** Returns true if gate in is HIGH */
    bool Gate();

    /** Set an LED (idx < 4) to a color */
    void SetLed(size_t idx, float red, float green, float blue);

    /** Get Knob Value, float from 0.0f to 1.0f */
    float GetKnobValue(int idx);

    /** Update LED PWM state. Call this once per main loop update to correctly display led colors */
    void UpdateLeds();

    /* Call this once per main loop update to test the Legio Hardware
     * Each input (1 gate in, 1 momentary in, 2 3-position switches, 3 controls) should change some or all of the output LEDs colors
     **/
    void UpdateExample();

    DaisySeed     seed;
    Encoder       encoder;
    GateIn        gate;
    RgbLed        leds[LED_LAST];
    AnalogControl controls[CONTROL_LAST];
    Switch3       sw[SW_LAST];

  private:
    void SetHidUpdateRates();
};

} // namespace daisy