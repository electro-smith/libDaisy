#include "hid_led.h"
using namespace daisy;
void Led::Init(dsy_gpio_pin pin, bool invert)
{
	// Init hardware LED
    // Simple OUTPUT GPIO for now.
    hw_pin_.pin = pin;
    hw_pin_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    dsy_gpio_init(&hw_pin_);
    // Set internal stuff.
    bright_  = 0.0f;
    pwm_cnt_ = 0;
    Set(bright_);
    invert_ = invert;
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
    bright_ = cube(val);
    pwm_thresh_ = bright_ * 255.0f;
}

void Led::Update()
{
    pwm_cnt_ = (pwm_cnt_ + 1) % 256;
    dsy_gpio_write(&hw_pin_, pwm_cnt_ < pwm_thresh_ ? on_ : off_);
}
