/** Class that handles initializing all of the hardware specific to the Daisy Patch Board.
*/
//
/** Helper funtions are also in place to provide easy access to built-in controls and peripherals.
*/
//
/** **Author:** Stephen Hensley
**Date Added:** November 2019
*/
#pragma once
#ifndef DSY_PATCH_BSP_H
#define DSY_PATCH_BSP_H
#include "daisy_seed.h"

namespace daisy
{
class DaisyPatch
{
  public:
    /** Enum of Ctrls to represent the four CV/Knob combos on the Patch
*/
    enum Ctrl
    {
        CTRL_1,
        CTRL_2,
        CTRL_3,
        CTRL_4,
        CTRL_LAST,
    };

    enum GateInput
    {
        GATE_IN_1,
        GATE_IN_2,
        GATE_IN_LAST,
    };


    DaisyPatch() {}
    ~DaisyPatch() {}

    /** Initializes the daisy seed, and patch hardware.
*/
    void Init();
    void DelayMs(size_t del);
    /** Audio Block size defaults to 48.
Change it using this function before StartingAudio
*/
    void SetAudioBlockSize(size_t size);
    void StartAudio(dsy_audio_mc_callback cb);
    void ChangeAudioCallback(dsy_audio_callback cb);
    void StartAdc();
    /** Hardware Accessors
*/
    float  AudioSampleRate();
    size_t AudioBlockSize();
    float  AudioCallbackRate();
    void   UpdateAnalogControls();
    float  GetCtrlValue(Ctrl k);
    void   DebounceControls();
    void   DisplayControls(bool invert = true);

    /** These are exposed for the user to access and manipulate directly
*/
    //
    /** Helper functions above provide easier access to much of what they are capable of.
*/
    DaisySeed     seed;
    Encoder       encoder;
    AnalogControl controls[CTRL_LAST];
    GateIn        gate_input[GATE_IN_LAST];
    MidiHandler   midi;
    OledDisplay   display;

    /** TODO: Add class for Gate output
*/
    dsy_gpio gate_output;


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
