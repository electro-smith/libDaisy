#pragma once
#ifndef DSY_GPIO_H
#define DSY_GPIO_H
#include "daisy_core.h"

namespace daisy
{

/** General Purpose IO driver */
class GpioHandle{
    public:

    struct Config{
        /** Which port the GPIO is on */
        typedef enum
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
        } dsy_gpio_port;

        /** Sets the mode of the GPIO */
        typedef enum
        {
            DSY_GPIO_MODE_INPUT,     /**< & */
            DSY_GPIO_MODE_OUTPUT_PP, /**< Push-Pull */
            DSY_GPIO_MODE_OUTPUT_OD, /**< Open-Drain */
            DSY_GPIO_MODE_ANALOG,    /**< & */
            DSY_GPIO_MODE_LAST,      /**< & */
        } dsy_gpio_mode;

        /** Configures whether an internal Pull up or Pull down resistor is used */
        typedef enum
        {
            DSY_GPIO_NOPULL,   /**< & */
            DSY_GPIO_PULLUP,   /**< & */
            DSY_GPIO_PULLDOWN, /**< & */
        } dsy_gpio_pull;

        uint8_t pin; /**< 0 - 15 */
        dsy_gpio_port  port;  /**< & */
        dsy_gpio_mode mode; /**< & */
        dsy_gpio_pull pull; /**< & */    
    };

    /** Return Values for the DacHandle class */
    enum class Result
    {
        OK,
        ERR,
    };

    /** Initializes the gpio with the settings configured. 
    \param *p Pin pointer
    */
    void dsy_gpio_init(const dsy_gpio *p);

    /** Deinitializes the gpio pin 
    \param *p Pin pointer
     */
    void dsy_gpio_deinit(const dsy_gpio *p);

    /** 
    Reads the state of the gpio pin
    \param *p Pin pointer 
    \return 1 if the pin is HIGH, and 0 if the pin is LOW */
    uint8_t dsy_gpio_read(const dsy_gpio *p);

    /** 
    Writes the state to the gpio pin
    Pin will be set to 3v3 when state is 1, and 0V when state is 0
    \param *p Pin pointer
    \param state State to write
    */
    void dsy_gpio_write(const dsy_gpio *p, uint8_t state);

    /** Toggles the state of the pin so that it is not at the same state as it was previously.
    \param *p Pin pointer
     */
    void dsy_gpio_toggle(const dsy_gpio *p);

    private:
        Config config_;
}
} // namespace daisy
