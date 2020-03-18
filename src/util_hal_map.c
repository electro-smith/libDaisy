#include "daisy_core.h"
#include "per_i2c.h"

#include "stm32h7xx_hal.h"

// Maps Daisy interface to STM32 HAL --
// I'd like to get all of this stuff tucked away somewhere inbetween the HAL, and User level
// So that I can start to slowly replace HAL stuff over time.
// Also I don't like that this throws a warning for every library file that doesn't use it...

// Possible solution: Move this to dsy_hal_interface.h -- and the explicitly include it in the lower-level files that need it.
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wunused-variable"
const static GPIO_TypeDef* gpio_hal_port_map[DSY_GPIO_LAST] = {
    GPIOA,
    GPIOB,
    GPIOC,
    GPIOD,
    GPIOE,
    GPIOF,
    GPIOG,
    GPIOH,
    GPIOI,
    GPIOJ,
    GPIOK,
    NULL,
};

const static uint16_t gpio_hal_pin_map[16] = {
    GPIO_PIN_0,
    GPIO_PIN_1,
    GPIO_PIN_2,
    GPIO_PIN_3,
    GPIO_PIN_4,
    GPIO_PIN_5,
    GPIO_PIN_6,
    GPIO_PIN_7,
    GPIO_PIN_8,
    GPIO_PIN_9,
    GPIO_PIN_10,
    GPIO_PIN_11,
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14,
    GPIO_PIN_15,
};
//#pragma GCC diagnostic pop

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;
I2C_HandleTypeDef hi2c4;


GPIO_TypeDef* dsy_hal_map_get_port(dsy_gpio_pin* p)
{
    return (GPIO_TypeDef*)gpio_hal_port_map[p->port];
}
uint16_t dsy_hal_map_get_pin(dsy_gpio_pin* p) {
    return (uint16_t)gpio_hal_pin_map[p->pin];
}

I2C_HandleTypeDef* dsy_hal_map_get_i2c(dsy_i2c_handle* p)
{
    I2C_HandleTypeDef* ptr[4] = {&hi2c1, &hi2c2, &hi2c3, &hi2c4};
    return ptr[p->periph];
}
