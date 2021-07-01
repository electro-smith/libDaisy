#include "daisy_core.h"

#include "stm32h7xx_hal.h"
#include "util/hal_map.h"

// Maps Daisy interface to STM32 HAL --
// I'd like to get all of this stuff tucked away somewhere inbetween the HAL, and User level
// So that I can start to slowly replace HAL stuff over time.
// Also I don't like that this throws a warning for every library file that doesn't use it...

using namespace daisy;

const static GPIO_TypeDef* gpio_hal_port_map[12] = {
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

// GPIO FUNCTIONS

GPIO_TypeDef* dsy_hal_map_get_port(const Pin pin)
{
    return (GPIO_TypeDef*)gpio_hal_port_map[(int)pin.port];
}
uint16_t dsy_hal_map_get_pin(const Pin pin)
{
    return (uint16_t)gpio_hal_pin_map[pin.pin];
}

void dsy_hal_map_gpio_clk_enable(const Pin pin)
{
    switch(pin.port)
    {
        case Pin::Port::DSY_GPIOA: __HAL_RCC_GPIOA_CLK_ENABLE(); return;
        case Pin::Port::DSY_GPIOB: __HAL_RCC_GPIOB_CLK_ENABLE(); return;
        case Pin::Port::DSY_GPIOC: __HAL_RCC_GPIOC_CLK_ENABLE(); return;
        case Pin::Port::DSY_GPIOD: __HAL_RCC_GPIOD_CLK_ENABLE(); return;
        case Pin::Port::DSY_GPIOE: __HAL_RCC_GPIOE_CLK_ENABLE(); return;
        case Pin::Port::DSY_GPIOF: __HAL_RCC_GPIOF_CLK_ENABLE(); return;
        case Pin::Port::DSY_GPIOG: __HAL_RCC_GPIOG_CLK_ENABLE(); return;
        case Pin::Port::DSY_GPIOH: __HAL_RCC_GPIOH_CLK_ENABLE(); return;
        case Pin::Port::DSY_GPIOI: __HAL_RCC_GPIOI_CLK_ENABLE(); return;
        case Pin::Port::DSY_GPIOJ: __HAL_RCC_GPIOJ_CLK_ENABLE(); return;
        case Pin::Port::DSY_GPIOK: __HAL_RCC_GPIOK_CLK_ENABLE(); return;
        default: return;
    }
}

// ADC FUNCTIONS
