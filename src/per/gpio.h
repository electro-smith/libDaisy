#pragma once
#ifndef DSY_GPIO_H
#define DSY_GPIO_H
#include "daisy_core.h"

namespace daisy
{
/** General Purpose IO driver */
class GPIO
{
  public:
    struct Config
    {
        /** Sets the mode of the GPIO */
        enum class Mode
        {
            INPUT,     /**< & */
            OUTPUT_PP, /**< Push-Pull */
            OUTPUT_OD, /**< Open-Drain */
            ANALOG,    /**< & */
        };

        /** Configures whether an internal Pull up or Pull down resistor is used */
        enum class Pull
        {
            NOPULL,   /**< & */
            PULLUP,   /**< & */
            PULLDOWN, /**< & */
        };

        enum class Speed
        {
            LOW,    /**< & */
            MEDIUM, /**< & */
            FAST,   /**< & */
            HIGH,   /**< & */
        };

        void Default()
        {
            mode  = Mode::INPUT;
            pull  = Pull::NOPULL;
            speed = Speed::LOW;
        }

        Mode         mode; /**< & */
        Pull         pull; /**< & */
        Speed        speed;
        dsy_gpio_pin pin; /**< & */
    };

    /** Initializes the gpio with the settings configured. 
    \param config Configuration for this pin
    */
    void Init(const Config config);

    /** Deinitializes the gpio pin 
     */
    void DeInit();

    /** 
    Reads the state of the gpio pin
    \return 1 if the pin is HIGH, and 0 if the pin is LOW */
    bool Read();

    /** 
    Writes the state to the gpio pin
    Pin will be set to 3v3 when state is 1, and 0V when state is 0
    \param state State to write
    */
    void Write(uint8_t state);

    /** Toggles the state of the pin so that it is not at the same state as it was previously.
     */
    void Toggle();

  private:
    Config config_;
};

} // namespace daisy
#endif