#pragma once
#ifndef DSY_HAL_MAP_H
#define DSY_HAL_MAP_H
#include "stm32h7xx_hal.h"
#include "daisy_core.h"

/** @addtogroup utility
    @{
*/
#ifdef __cplusplus
namespace daisy
{
constexpr uint32_t GetHALPin(Pin p)
{
    return (1 << p.pin);
}
constexpr GPIO_TypeDef *GetHALPort(Pin p)
{
    switch(p.port)
    {
        case PORTA: return GPIOA;
        case PORTB: return GPIOB;
        case PORTC: return GPIOC;
        case PORTD: return GPIOD;
        case PORTE: return GPIOE;
        case PORTF: return GPIOF;
        case PORTG: return GPIOG;
        case PORTH: return GPIOH;
        case PORTI: return GPIOI;
        case PORTJ: return GPIOJ;
        case PORTK: return GPIOK;
        default: return NULL;
    }
}
inline void GPIOClockEnable(Pin p)
{
    switch(p.port)
    {
        case PORTA: __HAL_RCC_GPIOA_CLK_ENABLE(); return;
        case PORTB: __HAL_RCC_GPIOB_CLK_ENABLE(); return;
        case PORTC: __HAL_RCC_GPIOC_CLK_ENABLE(); return;
        case PORTD: __HAL_RCC_GPIOD_CLK_ENABLE(); return;
        case PORTE: __HAL_RCC_GPIOE_CLK_ENABLE(); return;
        case PORTF: __HAL_RCC_GPIOF_CLK_ENABLE(); return;
        case PORTG: __HAL_RCC_GPIOG_CLK_ENABLE(); return;
        case PORTH: __HAL_RCC_GPIOH_CLK_ENABLE(); return;
        case PORTI: __HAL_RCC_GPIOI_CLK_ENABLE(); return;
        case PORTJ: __HAL_RCC_GPIOJ_CLK_ENABLE(); return;
        case PORTK: __HAL_RCC_GPIOK_CLK_ENABLE(); return;
        default: return;
    }
}
} // namespace daisy

extern "C"
{
#endif

    /** global structs, and helper functions for interfacing with the stm32 HAL library
    while it remains a dependancy.
    This file should only be included from source files (c/cpp)
    Including it from a header within libdaisy would expose the entire HAL to the users.
    This should be an option for users, but should not be required.
*/

    /** \param  *p Pin pin to get
    \return HAL GPIO_TypeDef as used in the HAL from a dsy_gpio_pin input. 
*/
    GPIO_TypeDef *dsy_hal_map_get_port(const dsy_gpio_pin *p);

    /** \param  *p Pin pin to get
    \return HAL GPIO Pin as used in the HAL from a dsy_gpio_pin input. 
*/
    uint16_t dsy_hal_map_get_pin(const dsy_gpio_pin *p);

    /** \param  port port clock to enable
*/
    void dsy_hal_map_gpio_clk_enable(dsy_gpio_port port);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
