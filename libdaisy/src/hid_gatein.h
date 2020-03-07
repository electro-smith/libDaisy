// # GateIn
// ## Description
//
// Generic Class for handling gate inputs through GPIO.
//
// ## Files
// hid_gatein.h
//
// ## Credits
//
// **Author:** Stephen Hensley
//
// **Date:** March 2020
//

#pragma once
#ifndef DSY_GATEIN_H
#define DSY_GATEIN_H
#include "per_gpio.h"

namespace daisy
{
class GateIn
{
  public:
    GateIn() {}
    ~GateIn() {}

    inline void Init(dsy_gpio *gatepin)
    {
        pin        = gatepin;
        prev_state = 0;
    }

    inline bool Trig()
    {
        bool new_state, out;
        new_state  = dsy_gpio_read(pin);
        out        = new_state && !prev_state;
        prev_state = new_state;
        return out;
    }

private:
    dsy_gpio *pin_;
    uint8_t   prev_state_;

};
} // namespace daisy
#endif
