#include "hid/gatein.h"

using namespace daisy;

void GateIn::Init(dsy_gpio_pin *pin_cfg, bool invert)
{
    pin_.pin    = *pin_cfg;
    pin_.mode   = DSY_GPIO_MODE_INPUT;
    pin_.pull   = DSY_GPIO_NOPULL;
    prev_state_ = 0;
    state_      = 0;
    invert_     = invert;
    dsy_gpio_init(&pin_);
}

bool GateIn::Trig()
{
    // Inverted because of typical BJT input circuit.
    prev_state_ = state_;
    state_      = invert_ ? !dsy_gpio_read(&pin_) : dsy_gpio_read(&pin_);
    return state_ && !prev_state_;
}