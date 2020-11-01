#pragma once
#ifndef DSY_DEV_SR_595_H
#define DSY_DEV_SR_595_H

#include "daisy_core.h"
#include "per/gpio.h"

const size_t kMaxSr595DaisyChain
    = 16; /**< Maximum Number of chained devices Connect device's QH' pin to the next chips serial input*/

/** @addtogroup shiftregister
    @{
    */

/**
   @brief Device Driver for 8-bit shift register. \n 
   CD74HC595 - 8-bit serial to parallel output shift
   @author shensley
   @date May 2020
*/
class ShiftRegister595
{
  public:
    /** The following pins correspond to the hardware connections
    to the 595. 
  */
    enum Pins
    {
        PIN_LATCH, /** LATCH corresonds to Pin 12 "RCLK" */
        PIN_CLK,   /** CLK corresponds to Pin 11 "SRCLK" */
        PIN_DATA,  /** DATA corresponds to Pin 14 "SER" */
        NUM_PINS, /** _SRCLR_ is not added here, but is tied to 3v3 on test hardware. */
    };
    ShiftRegister595() {}
    ~ShiftRegister595() {}

    /** 
    Initializes the GPIO, and data for the ShiftRegister
     * \param pin_cfg is an array of dsy_gpio_pin corresponding the the Pins enum above.
     * \param num_daisy_chained (default = 1) is the number of 595 devices daisy chained together.
     */
    void Init(dsy_gpio_pin *pin_cfg, size_t num_daisy_chained = 1);

    /** Sets the state of the specified output.
        \param idx The index starts with QA on the first device and ends with QH on the last device.
    \param state A true state will set the output HIGH, while a false state will set the output LOW.
    */
    void Set(uint8_t idx, bool state);

    /** Writes the states of shift register out to the connected devices.
     */
    void Write();

  private:
    dsy_gpio pin_[NUM_PINS];
    uint8_t  state_[kMaxSr595DaisyChain];
    size_t   num_devices_;
};

#endif
/** @} */
