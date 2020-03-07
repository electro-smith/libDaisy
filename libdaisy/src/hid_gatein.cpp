#include "hid_gatein.h"

using namespace daisy;

void GateIn::Init(dsy_gpio *gatepin)
{
    pin_        = gatepin;
    prev_state_ = 0;
}

bool GateIn::Trig()
{
    bool new_state, out;
    new_state  = dsy_gpio_read(pin_);
    out        = new_state && !prev_state_;
    prev_state_ = new_state;
    return out;
}