#pragma once
#ifndef DSY_POD_BSP_H
#define DSY_POD_BSP_H

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
class DaisyPod
{
  public:
    /** Switches */
    enum Sw
    {
        BUTTON_1,    /** & */
        BUTTON_2,    /** & */
        BUTTON_LAST, /** &  */
    };

    /** Knobs */
    enum Knob
    {
        KNOB_1,    /** &  */
        KNOB_2,    /** & */
        KNOB_LAST, /** & */
    };

    DaisyPod() {}
    ~DaisyPod() {}

    /** Init related stuff. */
    void Init();

    /** Wait for a bit
    \param del Time to wait in ms.
    */
    void DelayMs(size_t del);

    /** Audio Block size defaults to 48. Change it using this function before StartingAudio. 
    \param size Block size to set.
     */
    void SetAudioBlockSize(size_t size);

    /** Starts the callback
    \cb Interleaved callback function
    */
    void StartAudio(dsy_audio_callback cb);

    /** Starts the callback
    \cb multichannel callback function
    */
    void StartAudio(dsy_audio_mc_callback cb);

    /**
       Switch callback functions
       \param cb New interleaved callback function.
    */
    void ChangeAudioCallback(dsy_audio_callback cb);

    /**
       Switch callback functions
       \param cb New multichannel callback function.
    */
    void ChangeAudioCallback(dsy_audio_mc_callback cb);

    /**
       Start analog to digital conversion.
     */
    void StartAdc();

    /** Get sample rate */
    float AudioSampleRate();
    /** Get block size */
    size_t AudioBlockSize();
    /** Get callback rate */
    float AudioCallbackRate(); /** */
    /** Call at same rate as analog reads for smooth reading.*/
    void UpdateAnalogControls(); /** */
    /** & */
    float GetKnobValue(Knob k);
    /** & */
    void DebounceControls();
    /** Reset Leds*/
    void ClearLeds();
    /** Update Leds to set colors*/
    void UpdateLeds();

    /** Public Members */
    DaisySeed     seed;        /**<# */
    Encoder       encoder;     /**< & */
    AnalogControl knob1,       /**< & */
        knob2,                 /**< & */
        *knobs[KNOB_LAST];     /**< & */
    Switch button1,            /**< & */
        button2,               /**< & */
        *buttons[BUTTON_LAST]; /**< & */
    RgbLed led1,               /**< & */
        led2;                  /**< & */

  private:
    void   InitButtons();
    void   InitEncoder();
    void   InitLeds();
    void   InitKnobs();
    float  sample_rate_, callback_rate_;
    size_t block_size_;
};

} // namespace daisy
#endif
