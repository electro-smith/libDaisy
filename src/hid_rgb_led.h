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

    void
    Init(dsy_gpio_pin red, dsy_gpio_pin green, dsy_gpio_pin blue, bool invert);

    void Set(float r, float g, float b);
    void SetColor(Color c);

    void Update();

  private:
    Led r_, g_, b_;
};
} // namespace daisy

#endif
