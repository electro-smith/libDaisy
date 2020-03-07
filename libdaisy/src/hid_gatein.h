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

    void Init(dsy_gpio *gatepin);

    bool Trig();

private:
    dsy_gpio *pin_;
    uint8_t   prev_state_;

};
} // namespace daisy
#endif
