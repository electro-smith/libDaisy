#pragma once
#ifndef DSY_GATEIN_H
#define DSY_GATEIN_H
#include "per/gpio.h"

namespace daisy
{
/**
   @brief Generic Class for handling gate inputs through GPIO.
   @author Stephen Hensley
   @date    March 2020
   @ingroup controls
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

    /** @brief Initializes the gate input with specified hardware pin
     *
     *  @param pin_cfg pointer to pin to initialize
     *  @param invert True if the pin state is HIGH when 0V is present
     *         at the input. False if input signal matches the pin state.
     *
     *  @note the default for invert is true because it is typical to use
     *  an inverting input circuit (e.g. a BJT circuit) for eurorack gate inputs.
     */
    void Init(dsy_gpio_pin *pin_cfg, bool invert = true);

    /** Trig
    Checks current state of gate input.

    @return True if the GPIO just transitioned.
    */
    bool Trig();

    /** State
    Checks current state of gate input (no state required)

    read function is inverted because of suggested BJT input circuit
    */
    inline bool State()
    {
        return invert_ ? !dsy_gpio_read(&pin_) : dsy_gpio_read(&pin_);
    }

  private:
    dsy_gpio pin_;
    uint8_t  prev_state_, state_;
    bool     invert_;
};
} // namespace daisy
#endif
