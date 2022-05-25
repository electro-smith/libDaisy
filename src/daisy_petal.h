#pragma once
#ifndef DSY_PETAL_H
#define DSY_PETAL_H /**< & */

#include "daisy_seed.h"

namespace daisy
{
/**
   @brief Helpers and hardware definitions for daisy petal.
   @ingroup boards
*/
class DaisyPetal
{
  public:
    /** Switches */
    enum Sw
    {
        SW_1,    /**< Footswitch */
        SW_2,    /**< Footswitch */
        SW_3,    /**< Footswitch */
        SW_4,    /**< Footswitch */
        SW_5,    /**< Toggle */
        SW_6,    /**< Toggle */
        SW_7,    /**< Toggle */
        SW_LAST, /**< Last enum item */
    };

    /** Knobs */
    enum Knob
    {
        KNOB_1,    /**< & */
        KNOB_2,    /**< & */
        KNOB_3,    /**< & */
        KNOB_4,    /**< & */
        KNOB_5,    /**< & */
        KNOB_6,    /**< & */
        KNOB_LAST, /**< & */
    };

    /** Leds in ringled */
    enum RingLed
    {
        RING_LED_1,   /**< & */
        RING_LED_2,   /**< & */
        RING_LED_3,   /**< & */
        RING_LED_4,   /**< & */
        RING_LED_5,   /**< & */
        RING_LED_6,   /**< & */
        RING_LED_7,   /**< & */
        RING_LED_8,   /**< & */
        RING_LED_LAST /**< & */
    };

    /** footswitch leds */
    enum FootswitchLed
    {
        FOOTSWITCH_LED_1,    /**< & */
        FOOTSWITCH_LED_2,    /**< & */
        FOOTSWITCH_LED_3,    /**< & */
        FOOTSWITCH_LED_4,    /**< & */
        FOOTSWITCH_LED_LAST, /**< & */
    };

    /** Constructor */
    DaisyPetal() {}
    /** Destructor */
    ~DaisyPetal() {}

    /** Initialize daisy petal */
    void Init(bool boost = false);

    /**
       Wait before moving on.
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

    /** Stops the audio if it is running. */
    void StopAudio();

    /** Updates the Audio Sample Rate, and reinitializes.
     ** Audio must be stopped for this to work.
     */
    void SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate);

    /** Returns the audio sample rate in Hz as a floating point number.
     */
    float AudioSampleRate();

    /** Sets the number of samples processed per channel by the audio callback.
       \param size Audio block size
     */
    void SetAudioBlockSize(size_t size);

    /** Returns the number of samples per channel in a block of audio. */
    size_t AudioBlockSize();

    /** Returns the rate in Hz that the Audio callback is called */
    float AudioCallbackRate();

    /** Start analog to digital conversion. */
    void StartAdc();

    /** Stops Transfering data from the ADC */
    void StopAdc();

    /** Call at the same frequency as controls are read for stable readings.*/
    void ProcessAnalogControls();

    /** Process Analog and Digital Controls */
    inline void ProcessAllControls()
    {
        ProcessAnalogControls();
        ProcessDigitalControls();
    }


    /** Get value per knob.
    \param k Which knob to get
    \return Floating point knob position.
    */
    float GetKnobValue(Knob k);

    /** & */
    float GetExpression();

    /** Process digital controls */
    void ProcessDigitalControls();

    /** Turn all leds off */
    void ClearLeds();

    /** Update Leds to values you had set. */
    void UpdateLeds();

    /**
       Set ring LED colors
       \param idx Index to set
       \param r Red value
       \param g Green value
       \param b Blue value
     */
    void SetRingLed(RingLed idx, float r, float g, float b);

    /**
       Set footswitch LED
       \param idx Led Index
       \param bright Brightness
     */
    void SetFootswitchLed(FootswitchLed idx, float bright);

    DaisySeed seed;    /**< & */
    Encoder   encoder; /**< & */

    AnalogControl knob[KNOB_LAST]; /**< & */
    AnalogControl expression;      /**< & */
    Switch        switches[SW_LAST] /**< & */;

    RgbLed ring_led[8];       /**< & */
    Led    footswitch_led[4]; /**< & */

  private:
    void SetHidUpdateRates();
    void InitSwitches();
    void InitEncoder();
    void InitLeds();
    void InitAnalogControls();

    inline uint16_t* adc_ptr(const uint8_t chn) { return seed.adc.GetPtr(chn); }

    LedDriverPca9685<2, true> led_driver_;
};

} // namespace daisy

#endif
