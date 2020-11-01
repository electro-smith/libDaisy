#include "hid/led.h"
#include "per/tim.h"

using namespace daisy;

#define RESOLUTION_MAX (65535)

void Led::Init(dsy_gpio_pin pin, bool invert, float samplerate)
{
    // Init hardware LED
    // Simple OUTPUT GPIO for now.
    hw_pin_.pin  = pin;
    hw_pin_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    dsy_gpio_init(&hw_pin_);
    // Set internal stuff.
    bright_  = 0.0f;
    pwm_cnt_ = 0;
    Set(bright_);
    invert_     = invert;
    samplerate_ = samplerate;
    if(invert_)
    {
        on_  = false;
        off_ = true;
    }
    else
    {
        on_  = true;
        off_ = false;
    }
}
void Led::Set(float val)
{
    bright_     = cube(val);
    pwm_thresh_ = bright_ * static_cast<float>(RESOLUTION_MAX);
}

void Led::Update()
{
    // Shout out to @grrwaaa for the quick fix for pwm
    pwm_ += 120.f / samplerate_;
    if(pwm_ > 1.f)
        pwm_ -= 1.f;
    dsy_gpio_write(&hw_pin_, bright_ > pwm_ ? on_ : off_);

    // Once we have a slower timer set up:
    // Right now its too fast.

    //    dsy_gpio_write(&hw_pin_,
    //                   (dsy_tim_get_tick() & RESOLUTION_MAX) < pwm_thresh_ ? on_
    //                                                                       : off_);
}
