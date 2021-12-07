#pragma once
#ifndef DSY_GPIO_H
#define DSY_GPIO_H
#include "daisy_core.h"

/** New C++ GPIO */
#ifdef __cplusplus

namespace daisy
{
/** @brief General Purpose I/O control */
class GPIO
{
  public:
    /** @brief Mode of operation for the specified GPIO */
    enum class Mode
    {
        INPUT,     /**< Input for reading state of pin */
        OUTPUT,    /**< Output w/ push-pull configuration */
        OUTPUT_OD, /**< Output w/ open-drain configuration */
        ANALOG,    /**< Analog for connection to ADC or DAC peripheral */
    };

    /** @brief Configures whether an internal Pull up or Pull down resistor is used. 
     *  
     * Internal Pull up/down resistors are typically 40k ohms, and will be between
     * 30k and 50k
     * 
     * When the Pin is configured in Analog mode, the pull up/down resistors are
     * disabled by hardware. 
     */
    enum class Pull
    {
        NOPULL,   /**< No pull up resistor */
        PULLUP,   /**< Internal pull up enabled */
        PULLDOWN, /**< Internal pull down enabled*/
    };

    /** @brief Output speed controls the drive strength, and slew rate of the pin */
    enum class Speed
    {
        LOW,
        MEDIUM,
        HIGH,
        VERY_HIGH,
    };

    /** @brief Configuration for a given GPIO */
    struct Config
    {
        Pin   pin;
        Mode  mode;
        Pull  pull;
        Speed speed;

        /** Constructor with no arguments will prepare an invalid GPIO set as
         *  an input, with no pullup. 
         */
        Config()
        : pin(), mode(Mode::INPUT), pull(Pull::NOPULL), speed(Speed::LOW)
        {
        }
    };

    GPIO() {}

    /** @brief Initialize the GPIO from a Configuration struct */
    void Init(const Config &cfg);

    /** @brief Initialize the GPIO with a Configuration struct, and explicit pin */
    void Init(Pin p, const Config &cfg);

    /** @brief Explicity initialize all configuration for the GPIO */
    void Init(Pin   p,
              Mode  m  = Mode::INPUT,
              Pull  pu = Pull::NOPULL,
              Speed sp = Speed::LOW);

    /** @brief Deinitializes the GPIO pin */
    void DeInit();

    /** @brief Reads the state of the GPIO.
     *  If a GPIO is configured in Config::Mode::ANALOG, this function will 
     *  always return false.
     */
    bool Read();

    /** @brief Changes the state of the GPIO hardware when configured as an OUTPUT. 
     *  @param state setting true writes an output HIGH, while setting false writes an output LOW.
     */
    void Write(bool state);

    /** @brief flips the current state of the GPIO. 
     *  If it was HIGH, it will go LOW, and vice versa.
     */
    void Toggle();

    /** Return a reference to the internal Config struct */
    Config &GetConfig() { return cfg_; }

  private:
    /** This will internally be cast to the 
     *  STM32H7 GPIO_Typedef* type, which 
     *  is just the base address to the
     *  specified GPIO register. 
     * 
     *  This prevents us needing to have an internal
     *  Impl class just to store the GPIO_Typedef* 
     */
    uint32_t *GetGPIOBaseRegister();

    /** Internal copy of the Configuration of the given pin */
    Config cfg_;

    /** Internal pointer to base address of relavent GPIO register */
    uint32_t *port_base_addr_;
};

} // namespace daisy

#endif

/** Old Style C API for GPIO is staying in place for a 
 *  few versions to support backwards compatibility.
 * 
 *  This should not be used for anything new.
 */

#ifdef __cplusplus
extern "C"
{
#endif

    /** General Purpose IO driver */

    /** @addtogroup other
    @{
    */

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

    /** Struct for holding the pin, and configuration */
    typedef struct
    {
        dsy_gpio_pin  pin;  /**< & */
        dsy_gpio_mode mode; /**< & */
        dsy_gpio_pull pull; /**< & */
    } dsy_gpio;

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

#ifdef __cplusplus
}
#endif

#endif
/** @} */
