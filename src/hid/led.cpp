#include "hid/led.h"
#include "per/tim.h"

using namespace daisy;

#define RESOLUTION_MAX (65535)

void Led::Init(Pin pin, bool invert, float samplerate)
{
    // Init hardware LED
    // Simple OUTPUT GPIO for now.
    hw_pin_.Init(pin, GPIO::Mode::OUTPUT);
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
    hw_pin_.Write(bright_ > pwm_ ? on_ : off_);
}
