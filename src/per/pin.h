#pragma once
#ifndef DSY_PIN_H
#define DSY_PIN_H
#include "daisy_core.h"

namespace daisy
{
struct Pin
{
    Pin() { Init(Port::DSY_GPIOX, 0); }
    ~Pin() {}

    /** Enums and a simple struct for defining a hardware pin on the MCU
        These correlate with the stm32 datasheet, and are used to configure
        the hardware.
        */
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

    //allow equality operator to be used, replaces pin_cmp
    bool operator==(Pin &rhs) { return (rhs.pin == pin) && (rhs.port == port); }

    void Init(Port init_port, uint8_t init_pin)
    {
        port = init_port;
        pin  = init_pin;
    }

    Port    port;
    uint8_t pin;
};
} // namespace daisy
#endif