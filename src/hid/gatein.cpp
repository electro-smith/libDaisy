#include "hid/gatein.h"

using namespace daisy;

void GateIn::Init(Pin pin_cfg)
{
    GPIO::Config gpio_config; //default constructor is ok
    gpio_config.pin = pin_cfg;
    gpio_.Init(gpio_config);

    prev_state_ = 0;
    state_      = 0;
}

bool GateIn::Trig()
{
    // Inverted because of typical BJT input circuit.
    prev_state_ = state_;
    state_      = !gpio_.Read();
    return state_ && !prev_state_;
}