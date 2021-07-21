#pragma once
#ifndef DSY_RGB_LED_H
#define DSY_RGB_LED_H
#include "hid/led.h"
#include "util/color.h"

namespace daisy
{
/** @addtogroup feedback
    @{
*/

/** 3x LEDs configured as an RGB for ease of use. */
class RgbLed
{
  public:
    RgbLed() {}
    ~RgbLed() {}

    /** Initializes 3x GPIO Pins as red, green, and blue elements of an RGB LED
    \param red  Red element
    \param green Green element
    \param blue Blue element
    \param invert Flips led polarity
    */
    void
    Init(dsy_gpio_pin red, dsy_gpio_pin green, dsy_gpio_pin blue, bool invert);

    /** Sets each element of the LED with a floating point number 0-1 
    \param r Red element
    \param g Green element
    \param b Blue element
     */
    void Set(float r, float g, float b);

    /** Sets the red channel of the LED with a floating point number 0-1
    \param val brightness of the red channel
    */
    void SetRed(float val);

    /** Sets the green channel of the LED with a floating point number 0-1
    \param val brightness of the green channel
    */
    void SetGreen(float val);

    /** Sets the blue channel of the LED with a floating point number 0-1
    \param val brightness of the blue channel
    */
    void SetBlue(float val);

    /** Sets the RGB using a Color object.
    \param c Color object to set.
     */
    void SetColor(Color c);

    /** Updates the PWM of the LED based on the current values.
    Should be called at a regular interval. (i.e. 1kHz/1ms)
    */
    void Update();

  private:
    Led r_, g_, b_;
};
/** @} */
} // namespace daisy

#endif
