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
    void Init(dsy_gpio *gatepin);
// ~~~~
// #### Trig
// Checks current state of gate input. 
// Returns FALSE if pin is low, and TRUE if high
//
// ~~~~
    bool Trig();
// ~~~~

private:
    dsy_gpio *pin_;
    uint8_t   prev_state_;

};
} // namespace daisy
#endif
