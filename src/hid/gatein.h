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
    /** GateIn Constructor */
    GateIn() {}

    /** GateIn Destructor */
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
    void Init(Pin pin, bool invert = true);

    /** @brief Initializes the gate input with specified hardware pin
     *
     *  @param pin_cfg pointer to pin to initialize
     *  @param invert True if the pin state is HIGH when 0V is present
     *         at the input. False if input signal matches the pin state.
     *
     *  @note the default for invert is true because it is typical to use
     *  an inverting input circuit (e.g. a BJT circuit) for eurorack gate inputs.
     * 
     *  @note deprectated - this function still works, but will eventually be removed.
     *  It uses the old style dsy_gpio_pin in a way that it is not compatible with the
     *  the new Pin class. 
     */
    void Init(dsy_gpio_pin *pin_cfg, bool invert = true);

    /** Checks current state of gate input.
     *  @return True if the GPIO just transitioned.
     */
    bool Trig();

    /** Checks current state of gate input (no state required)
     *  read function is inverted because of suggested BJT input circuit
     */
    inline bool State() { return invert_ ? !pin_.Read() : pin_.Read(); }

  private:
    GPIO pin_;
    bool prev_state_, state_;
    bool invert_;
};
} // namespace daisy
#endif
