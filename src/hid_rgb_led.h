/** 3x LEDs configured as an RGB for ease of use.

*/
#pragma once
#ifndef DSY_RGB_LED_H
#define DSY_RGB_LED_H
#include "hid_led.h"
#include "util_color.h"

namespace daisy
{
class RgbLed
{
  public:
    RgbLed() {}
    ~RgbLed() {}

    /** Initializes 3x GPIO Pins as red, green, and blue elements of an RGB LED

Invert will flip polarity of LED.
*/
    void
    Init(dsy_gpio_pin red, dsy_gpio_pin green, dsy_gpio_pin blue, bool invert);

    /** Sets each element of the LED with a floating point number 0-1
*/
    void Set(float r, float g, float b);

    /** Sets the RGB using a Color object.
*/
    void SetColor(Color c);

    /** Updates the PWM of the LED based on the current values.
*/
    //
    /** Should be called at a regular interval. (i.e. 1kHz/1ms)
*/
    void Update();

  private:
    Led r_, g_, b_;
};
} // namespace daisy

#endif
