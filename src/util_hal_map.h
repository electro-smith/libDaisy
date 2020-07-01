#pragma once
#ifndef DSY_HAL_MAP_H
#define DSY_HAL_MAP_H
#include "stm32h7xx_hal.h"
#include "daisy_core.h"
#include "per_i2c.h"

/** @addtogroup utility
    @{
*/


/** global structs, and helper functions for interfacing with the stm32 HAL library
    while it remains a dependancy.
    This file should only be included from source files (c/cpp)
    Including it from a header within libdaisy would expose the entire HAL to the users.
    This should be an option for users, but should not be required.
*/

extern I2C_HandleTypeDef hi2c1; /**< externs of HAL handles... */
extern I2C_HandleTypeDef hi2c2; /**< externs of HAL handles... */
extern I2C_HandleTypeDef hi2c3; /**< externs of HAL handles... */
extern I2C_HandleTypeDef hi2c4; /**< externs of HAL handles... */


/** \param  *p Pin pin to get
    \return HAL GPIO_TypeDef as used in the HAL from a dsy_gpio_pin input. 
*/
GPIO_TypeDef *dsy_hal_map_get_port(const dsy_gpio_pin *p);

/** \param  *p Pin pin to get
    \return HAL GPIO Pin as used in the HAL from a dsy_gpio_pin input. 
*/
uint16_t dsy_hal_map_get_pin(const dsy_gpio_pin *p);


/** \param  *p dsy_i2c_handle to get
    \return The I2C_HandleTypeDef for the given *p
*/
I2C_HandleTypeDef *dsy_hal_map_get_i2c(const dsy_i2c_handle *p);

#endif
/** @} */
