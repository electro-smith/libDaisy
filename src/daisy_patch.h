#pragma once
#ifndef DSY_PATCH_BSP_H
#define DSY_PATCH_BSP_H
#include "daisy_seed.h"
#include "dev/oled_ssd130x.h"

namespace daisy
{
/**
    @brief Class that handles initializing all of the hardware specific to the Daisy Patch Board. \n 
    Helper funtions are also in place to provide easy access to built-in controls and peripherals.
    @author Stephen Hensley
    @date November 2019
    @ingroup boards
*/
class DaisyPatch
{
  public:
    /** Enum of Ctrls to represent the four CV/Knob combos on the Patch
     */
    enum Ctrl
    {
        CTRL_1,    /**< */
        CTRL_2,    /**< */
        CTRL_3,    /**< */
        CTRL_4,    /**< */
        CTRL_LAST, /**< */
    };

    /** Daisy patch gate inputs */
    enum GateInput
    {
        GATE_IN_1,    /**< */
        GATE_IN_2,    /** <*/
        GATE_IN_LAST, /**< */
    };

    /** Constructor */
    DaisyPatch() {}
    /** Destructor */
    ~DaisyPatch() {}

    /** Initializes the daisy seed, and patch hardware.*/
    void Init(bool boost = false);

    /** 
    Wait some ms before going on.
    \param del Delay time in ms.
    */
    void DelayMs(size_t del);


    /** Starts the callback
    \param cb multichannel callback function
    */
    void StartAudio(AudioHandle::AudioCallback cb);

    /**
       Switch callback functions
       \param cb New multichannel callback function.
    */
    void ChangeAudioCallback(AudioHandle::AudioCallback cb);

    /** Stops the audio */
    void StopAudio();

    /** Set the sample rate for the audio */
    void SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate);

    /** Get sample rate */
    float AudioSampleRate();

    /** Audio Block size defaults to 48.
    Change it using this function before StartingAudio
    \param size Audio block size.
    */
    void SetAudioBlockSize(size_t size);

    /** Returns the number of samples per channel in a block of audio. */
    size_t AudioBlockSize();

    /** Returns the rate in Hz that the Audio callback is called */
    float AudioCallbackRate();

    /** Start analog to digital conversion.*/
    void StartAdc();

    /** Stops Transfering data from the ADC */
    void StopAdc();


    /** Call at same rate as reading controls for good reads. */
    void ProcessAnalogControls();

    /** Process Analog and Digital Controls */
    inline void ProcessAllControls()
    {
        ProcessAnalogControls();
        ProcessDigitalControls();
    }

    /**
       Get value for a particular control
       \param k Which control to get
     */
    float GetKnobValue(Ctrl k);

    /**  Process the digital controls */
    void ProcessDigitalControls();

    /**  Control the display */
    void DisplayControls(bool invert = true);

    /* These are exposed for the user to access and manipulate directly
       Helper functions above provide easier access to much of what they are capable of.
    */
    DaisySeed       seed;                             /**< Seed object */
    Encoder         encoder;                          /**< Encoder object */
    AnalogControl   controls[CTRL_LAST];              /**< Array of controls*/
    GateIn          gate_input[GATE_IN_LAST];         /**< Gate inputs  */
    MidiUartHandler midi;                             /**< Handles midi*/
    OledDisplay<SSD130x4WireSpi128x64Driver> display; /**< & */

    // TODO: Add class for Gate output
    dsy_gpio gate_output; /**< &  */


  private:
    void SetHidUpdateRates();
    void InitAudio();
    void InitControls();
    void InitDisplay();
    void InitMidi();
    void InitCvOutputs();
    void InitEncoder();
    void InitGates();

    uint32_t screen_update_last_, screen_update_period_;
};

} // namespace daisy

#endif
