#pragma once
#ifndef DSY_GPIO_H
#define DSY_GPIO_H
#include "daisy_core.h"

namespace daisy
{
enum class Port
{
    DSY_GPIOA, /**< & */
    DSY_GPIOB, /**< & */
    DSY_GPIOC, /**< & */
    DSY_GPIOD, /**< & */
    DSY_GPIOE, /**< & */
    DSY_GPIOF, /**< & */
    DSY_GPIOG, /**< & */
    DSY_GPIOH, /**< & */
    DSY_GPIOI, /**< & */
    DSY_GPIOJ, /**< & */
    DSY_GPIOK, /**< & */
    DSY_GPIOX, /** This is a non-existant port for unsupported bits of hardware. */
    DSY_GPIO_LAST, /** Final enum member */
};

struct Pin
{
    Port    port = Port::DSY_GPIOX;
    uint8_t pin  = 0;

    //allow equality operator to be used, replaces pin_cmp
    bool operator==(Pin &rhs) { return (rhs.pin == pin) && (rhs.port == port); }
};

/** General Purpose IO driver */
class GPIO
{
  public:
    GPIO() {}
    ~GPIO() {}

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

        Config()
        {
            //default setup is input
            mode      = Mode::INPUT;
            pull      = Pull::NOPULL;
            speed     = Speed::LOW;
            alternate = 0;
        }

        Mode     mode; /**< & */
        Pull     pull; /**< & */
        Speed    speed;
        Pin      pin;
        uint32_t alternate;
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
    void Write(bool state);

    /** Toggles the state of the pin so that it is not at the same state as it was previously.
     */
    void Toggle();

  private:
    Pin pin_;
};
} // namespace daisy
#endif