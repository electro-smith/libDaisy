#pragma once
#ifndef DSY_HAL_MAP_H
#define DSY_HAL_MAP_H
#include "stm32h7xx_hal.h"
#include "daisy_core.h"

/** @addtogroup utility
    @{
*/


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

#endif
/** @} */
