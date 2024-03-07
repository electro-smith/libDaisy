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

#endif
#endif
/** @} */
