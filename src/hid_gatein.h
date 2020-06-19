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

    // ## General Functions
    // ### Init
    // Initializes the gate input with specified hardware pin
    //
    // ~~~~
    void Init(dsy_gpio_pin *pin_cfg);
    // ~~~~

    // #### Trig
    // Checks current state of gate input.
    // Returns True if the GPIO just transitioned.
    //
    // ~~~~
    bool Trig();
    // ~~~~

    // #### State
    // Checks current state of gate input (no state required)
    //
    // read function is inverted because of suggested BJT input circuit
    //
    // ~~~~
    inline bool State() { return !dsy_gpio_read(&pin_); }
    // ~~~~


  private:
    dsy_gpio pin_;
    uint8_t  prev_state_, state_;
};
} // namespace daisy
#endif
