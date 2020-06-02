#pragma once
#ifndef DSY_GATEIN_H
#define DSY_GATEIN_H
#include "per_gpio.h"

namespace daisy
{

  /** 
    hid_gatein.h 
    @brief Generic Class for handling gate inputs through GPIO.
    @author Stephen Hensley
    @date    March 2020
  */
class GateIn
{
  public:
    /** GateIn 
	Constructor 
    */
    GateIn() {}
    /** GateIn~
	Destructor 
    */
    ~GateIn() {}

    /** Init
	Initializes the gate input with specified hardware pin
    */
    void Init(dsy_gpio_pin *pin_cfg);

    /** Trig
	Checks current state of gate input.
	\return FALSE if pin is low, and TRUE if high
    */
    bool Trig();

  private:
    dsy_gpio pin_;
    uint8_t  prev_state_, state_;
};
} // namespace daisy
#endif
