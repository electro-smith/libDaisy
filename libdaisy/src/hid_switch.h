// # Switch
// ## Description
//
// Generic Class for handling momentary/latching switches
// 
// ## Files
// hid_switch.*
//
// ## Credits
//
// **Author:** Stephen Hensley
//
// **Date:** December 2019
//
// Inspired/influenced by Mutable Instruments (pichenettes) Switch classes
//
#pragma once
#ifndef DSY_SWITCH_H
#define DSY_SWITCH_H
#include "daisy_core.h"
#include "per_gpio.h"

namespace daisy
{
class Switch
{
  public:
// ## Data Types
// ### Type
// Specifies the expected behavior of the switch
// ~~~~
    enum Type
    {
        TYPE_TOGGLE,
        TYPE_MOMENTARY,
    };
// ~~~~
// ### Polarity
// Specifies whether the pressed is HIGH or LOW.
// ~~~~
    enum Polarity
    {
        POLARITY_NORMAL,
        POLARITY_INVERTED,
    };
// ~~~~
// ### Pull
// Specifies whether to use built-in Pull Up/Down resistors to hold button
// at a given state when not engaged.
// ~~~~
    enum Pull
    {
        PULL_UP,
        PULL_DOWN,
        PULL_NONE,
    };
// ~~~~

    Switch() {}

    ~Switch() {}

// ## General Functions
// ### Init
// Initializes the switch object with a given port/pin combo.
//
// Parameters:
// - pin: port/pin object to tell the switch which hardware pin to use.
// - update_rate: the rate at which the Debounce() function will be called. (used for timing).
// - t: switch type -- Default: TYPE_MOMENTARY
// - pol: switch polarity -- Default: POLARITY_INVERTED
// - pu: switch pull up/down -- Default: PULL_UP
// ~~~~
    void
         Init(dsy_gpio_pin pin, float update_rate, Type t, Polarity pol, Pull pu);
    void Init(dsy_gpio_pin pin, float update_rate);
// ~~~~

// ### Debounce
// Called at update_rate to debounce and handle timing for the switch.
//
// In order for events not to be missed, its important that the Edge/Pressed checks
// be made at the same rate as the debounce function is being called.
// ~~~~
    void Debounce();
// ~~~~

// ### RisingEdge
// Returns true if a button was just pressed.
// ~~~~
    inline bool RisingEdge() const { return state_ == 0x7f; }
// ~~~~

// ### FallingEdge
// Returns true if the button was just released
// ~~~~
    inline bool FallingEdge() const { return state_ == 0x80; }
// ~~~~

// ### Pressed
// Returns true if the button is held down (or if the toggle is on).
// ~~~~
    inline bool Pressed() const { return state_ == 0xff; }
// ~~~~

// ### TimeHeldMs
// Returns the time in milliseconds that the button has been held (or toggle has been on)
// ~~~~
    inline float TimeHeldMs() const
    // ~~~~
    {
        return Pressed() ? time_held_ * 1000.0f : 0;
    }

  private:
    Type     t;
    dsy_gpio hw_gpio_;
    uint8_t  state_;
    bool     flip_;
    float    time_per_update_, time_held_;
};

} // namespace daisy
#endif
