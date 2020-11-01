#include "hid/switch.h"
using namespace daisy;

void Switch::Init(dsy_gpio_pin pin,
                  float        update_rate,
                  Type         t,
                  Polarity     pol,
                  Pull         pu)
{
    time_per_update_ = 1.0f / update_rate;
    state_           = 0x00;
    time_held_       = 0;
    t_               = t;
    // Flip may seem opposite to logical direction,
    // but here 1 is pressed, 0 is not.
    flip_         = pol == POLARITY_INVERTED ? true : false;
    hw_gpio_.pin  = pin;
    hw_gpio_.mode = DSY_GPIO_MODE_INPUT;
    switch(pu)
    {
        case PULL_UP: hw_gpio_.pull = DSY_GPIO_PULLUP; break;
        case PULL_DOWN: hw_gpio_.pull = DSY_GPIO_PULLDOWN; break;
        case PULL_NONE: hw_gpio_.pull = DSY_GPIO_NOPULL; break;
        default: hw_gpio_.pull = DSY_GPIO_PULLUP; break;
    }
    dsy_gpio_init(&hw_gpio_);
}
void Switch::Init(dsy_gpio_pin pin, float update_rate)
{
    Init(pin, update_rate, TYPE_MOMENTARY, POLARITY_INVERTED, PULL_UP);
}

void Switch::Debounce()
{
    // shift over, and introduce new state.
    state_ = (state_ << 1)
             | (flip_ ? !dsy_gpio_read(&hw_gpio_) : dsy_gpio_read(&hw_gpio_));
    // Reset time held on any edge.
    if(state_ == 0x7f || state_ == 0x80)
        time_held_ = 0;
    // Add while held (8-tick delay on hold due to debouncing).
    if(state_ == 0xff)
        time_held_ += time_per_update_;
}
