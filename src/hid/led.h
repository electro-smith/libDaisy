
#pragma once
#ifndef DSY_LED_H
#define DSY_LED_H
#include "daisy_core.h"
#include "per/gpio.h"

/* TODO - Get this set up to work with the dev_leddriver stuff as well
- Setup Hardware PWM for pins that have it
*/

namespace daisy
{
/**
    @brief LED Class providing simple Software PWM ability, etc \n 
    Eventually this will work with hardware PWM, and external LED Driver devices as well.
    @author shensley
    @date March 2020
    @ingroup feedback
*/
class Led
{
  public:
    Led() {}
    ~Led() {}

    /** 
    Initializes an LED using the specified hardware pin.
    \param pin chooses LED pin
    \param invert will set whether to internally invert the brightness due to hardware config.
        \param samplerate sets the rate at which 'Update()' will be called (used for software PWM)
    */
    void Init(dsy_gpio_pin pin, bool invert, float samplerate = 1000.0f);

    /** 
    Sets the brightness of the Led.
    \param val will be cubed for gamma correction, and then quantized to 8-bit values for Software PWM
    8-bit is for more flexible update rate options, as 12-bit or more would require faster update rates.
    */
    void Set(float val);

    /** 
    This processes the pwm of the LED
    sets the hardware accordingly.
    */
    void Update();

    /** Set the rate at which you'll update the leds without reiniting the led
     *  \param sample_rate New update rate in hz.
    */
    inline void SetSampleRate(float sample_rate) { samplerate_ = sample_rate; }

  private:
    size_t   pwm_cnt_, pwm_thresh_;
    float    bright_;
    float    pwm_;
    float    samplerate_;
    bool     invert_, on_, off_;
    dsy_gpio hw_pin_;
};

} // namespace daisy

#endif
