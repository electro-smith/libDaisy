#pragma once
#ifndef DSY_PATCH_BSP_H
#define DSY_PATCH_BSP_H
#include "daisy_seed.h"

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
    void Init();

    /** 
    Wait some ms before going on.
    \param del Delay time in ms.
    */
    void DelayMs(size_t del);

    /** Audio Block size defaults to 48.
    Change it using this function before StartingAudio
    \param size Audio block size.
    */
    void SetAudioBlockSize(size_t size);

    /** Starts the callback
    \cb multichannel callback function
    */
    void StartAudio(dsy_audio_mc_callback cb);

    /**
       Switch callback functions
       \param cb New multichannel callback function.
    */
    void ChangeAudioCallback(dsy_audio_mc_callback cb);

    /** Start analog to digital conversion.*/
    void StartAdc();

    // Hardware Accessors
    /** Get sample rate */
    float AudioSampleRate();
    /** Get block size */
    size_t AudioBlockSize();
    /** Get callback rate */
    float AudioCallbackRate();
    /** Call at same rate as reading controls for good reads. */
    void UpdateAnalogControls();
    /**
       Get value for a partiular control
       \param k Which control to get
     */
    float GetCtrlValue(Ctrl k);

    /** Debounce analog controls. Call at same rate as reading controls.*/
    void DebounceControls();

    /**  Control the display */
    void DisplayControls(bool invert = true);

    /* These are exposed for the user to access and manipulate directly
       Helper functions above provide easier access to much of what they are capable of.
    */
    DaisySeed     seed;                     /**< Seed object */
    Encoder       encoder;                  /**< Encoder object */
    AnalogControl controls[CTRL_LAST];      /**< Array of controls*/
    GateIn        gate_input[GATE_IN_LAST]; /**< Gate inputs  */
    MidiHandler   midi;                     /**< Handles midi*/
    OledDisplay   display;                  /**< & */

    // TODO: Add class for Gate output
    dsy_gpio gate_output; /**< &  */


  private:
    void InitAudio();
    void InitControls();
    void InitDisplay();
    void InitMidi();
    void InitCvOutputs();
    void InitEncoder();
    void InitGates();

    size_t   block_size_;
    dsy_gpio ak4556_reset_pin_;
    uint32_t screen_update_last_, screen_update_period_;
};

} // namespace daisy

#endif
