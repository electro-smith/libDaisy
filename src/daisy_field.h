#pragma once
#ifndef DSY_FIELD_BSP_H
#define DSY_FIELD_BSP_H /**< & */
#include "daisy_seed.h"
#include "dev/oled_ssd130x.h"

/**
   @brief Hardware defines and helpers for daisy field platform.
*/
namespace daisy
{
/** @addtogroup boards
    @{
*/
class DaisyField
{
  public:
    /** enums for controls, etc.
*/
    enum
    {
        SW_1,    /**< tactile switch */
        SW_2,    /**< tactile switch */
        SW_LAST, /**< & */
    };

    /** All knobs connect to Daisy Seed's ADC1 pin via CD4051 mux
        Knobs are in order that they are laid out on hardware.
    */
    enum
    {
        KNOB_1,    /**< & */
        KNOB_2,    /**< & */
        KNOB_3,    /**< & */
        KNOB_4,    /**< & */
        KNOB_5,    /**< & */
        KNOB_6,    /**< & */
        KNOB_7,    /**< & */
        KNOB_8,    /**< & */
        KNOB_LAST, /**< & */
    };

    enum
    {
        CV_1,    /** Connected to ADC1_INP17 */
        CV_2,    /** Connected  to ADC1_INP15 */
        CV_3,    /** Connected to ADC1_INP4 */
        CV_4,    /** Connected to ADC1_INP11 */
        CV_LAST, /**< & */
    };

    enum
    {
        LED_KEY_B1, /**< & */
        LED_KEY_B2, /**< & */
        LED_KEY_B3, /**< & */
        LED_KEY_B4, /**< & */
        LED_KEY_B5, /**< & */
        LED_KEY_B6, /**< & */
        LED_KEY_B7, /**< & */
        LED_KEY_B8, /**< & */
        LED_KEY_A8, /**< & */
        LED_KEY_A7, /**< & */
        LED_KEY_A6, /**< & */
        LED_KEY_A5, /**< & */
        LED_KEY_A4, /**< & */
        LED_KEY_A3, /**< & */
        LED_KEY_A2, /**< & */
        LED_KEY_A1, /**< & */
        LED_KNOB_1, /**< & */
        LED_KNOB_2, /**< & */
        LED_KNOB_3, /**< & */
        LED_KNOB_4, /**< & */
        LED_KNOB_5, /**< & */
        LED_KNOB_6, /**< & */
        LED_KNOB_7, /**< & */
        LED_KNOB_8, /**< & */
        LED_SW_1,   /**< & */
        LED_SW_2,   /**< & */
        LED_LAST    /**< & */
    };

    DaisyField() {}
    ~DaisyField() {}

    /**Initializes the Daisy Field, and all of its hardware.*/
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
    \param cb multichannel callback function
    */
    void StartAudio(AudioHandle::AudioCallback cb);

    /** Stops the audio if it is running. */
    void StopAudio();

    /**
       Switch callback functions
       \param cb New interleaved callback function.
    */
    void ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb);

    /**
       Switch callback functions
       \param cb New multichannel callback function.
    */
    void ChangeAudioCallback(AudioHandle::AudioCallback cb);

    /** Updates the Audio Sample Rate, and reinitializes.
     ** Audio must be stopped for this to work.
     */
    void SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate);

    /** Returns the audio sample rate in Hz as a floating point number.
     */
    float AudioSampleRate();

    /** Sets the number of samples processed per channel by the audio callback.
     */
    void SetAudioBlockSize(size_t blocksize);

    /** Returns the number of samples per channel in a block of audio. */
    size_t AudioBlockSize();

    /** Returns the rate in Hz that the Audio callback is called */
    float AudioCallbackRate();

    /** Starts Transfering data from the ADC */
    void StartAdc();

    /** Stops Transfering data from the ADC */
    void StopAdc();

    /** Turns on the built-in 12-bit DAC on the Daisy Seed */
    /** **This is now deprecated and does nothing.** 
     ** The polling use of the DACs now handles starting the tranmission.  */
    void StartDac();

    /** Processes the ADC inputs, updating their values */
    void ProcessAnalogControls();

    /** Process tactile switches and keyboard states */
    void ProcessDigitalControls();

    /** Process Analog and Digital Controls */
    inline void ProcessAllControls()
    {
        ProcessAnalogControls();
        ProcessDigitalControls();
    }

    /** Sets the output of CV out 1 to a value between 0-4095 that corresponds to 0-5V */
    void SetCvOut1(uint16_t val);

    /** Sets the output of CV out 2 to a value between 0-4095 that corresponds to 0-5V */
    void SetCvOut2(uint16_t val);

    /** Returns true if the key has not been pressed recently
        \param idx the key of interest
    */
    bool KeyboardState(size_t idx) const;

    /** Returns true if the key has just been pressed
        \param idx the key of interest
    */
    bool KeyboardRisingEdge(size_t idx) const;

    /** Returns true if the key has just been released
        \param idx the key of interest
    */
    bool KeyboardFallingEdge(size_t idx) const;

    /** Returns the knob's value
        \param idx The knob of interest.
    */
    float GetKnobValue(size_t idx) const;

    /** Returns the CV input's value
        \param idx The CV input of interest.
    */
    float GetCvValue(size_t idx) const;

    /** Getter for switch objects
        \param idx The switch of interest.
    */
    Switch* GetSwitch(size_t idx);

    /** Getter for knob objects
        \param idx The knob input of interest.
    */
    AnalogControl* GetKnob(size_t idx);

    /** Getter for CV objects.
        \param idx The CV input of interest.
    */
    AnalogControl* GetCv(size_t idx);

    /**Light show, cycling through all LEDs, and OLED
     **/
    void VegasMode();

    DaisySeed                                seed;
    OledDisplay<SSD130x4WireSpi128x64Driver> display;
    dsy_gpio                                 gate_out;
    GateIn                                   gate_in;
    LedDriverPca9685<2, true>                led_driver;
    Switch                                   sw[SW_LAST];
    AnalogControl                            knob[KNOB_LAST];
    AnalogControl                            cv[CV_LAST];
    MidiUartHandler                          midi;

  private:
    /** Set all the HID callback rates any time a new callback rate is established */
    void SetHidUpdateRates();
    void InitMidi();

    ShiftRegister4021<2> keyboard_sr_; /**< Two 4021s daisy-chained. */
    uint8_t              keyboard_state_[16];
    uint32_t             last_led_update_; // for vegas mode
    bool                 gate_in_trig_;    // True when triggered.
};

/** @} */

} // namespace daisy

#endif
