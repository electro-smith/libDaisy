#include "hid/switch.h"
using namespace daisy;

void Switch::Init(Pin pin, float update_rate, Type t, Polarity pol, Pull pu)
{
    time_per_update_ = 1.0f / update_rate;
    state_           = 0x00;
    time_held_       = 0;
    t_               = t;
    // Flip may seem opposite to logical direction,
    // but here 1 is pressed, 0 is not.
    flip_ = pol == POLARITY_INVERTED ? true : false;

    GPIO::Config gpio_conf;
    gpio_conf.pin = pin;

    switch(pu)
    {
        case PULL_UP: gpio_conf.pull = GPIO::Config::Pull::PULLUP; break;
        case PULL_DOWN: gpio_conf.pull = GPIO::Config::Pull::PULLDOWN; break;
        case PULL_NONE: gpio_conf.pull = GPIO::Config::Pull::NOPULL; break;
        default: gpio_conf.pull = GPIO::Config::Pull::PULLUP; break;
    }

    hw_gpio_.Init(gpio_conf);
}
void Switch::Init(Pin pin, float update_rate)
{
    Init(pin, update_rate, TYPE_MOMENTARY, POLARITY_INVERTED, PULL_UP);
}

void Switch::Debounce()
{
    // shift over, and introduce new state.
    state_ = (state_ << 1)
             | (flip_ ? !hw_gpio_.Read() : hw_gpio_.Read();
    // Reset time held on any edge.
    if(state_ == 0x7f || state_ == 0x80)
        time_held_ = 0;
    // Add while held (8-tick delay on hold due to debouncing).
    if(state_ == 0xff)
        time_held_ += time_per_update_;
}
