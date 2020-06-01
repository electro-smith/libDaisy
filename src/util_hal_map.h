//
/** global structs, and helper functions for interfacing with the stm32 HAL library
while it remains a dependancy.
*/
//
//
/** This file should only be included from source files (c/cpp)
*/
//
/** Including it from a header within libdaisy would expose the entire HAL to the users.

This should be an option for users, but should not be required.
*/
//
#pragma once
#ifndef DSY_HAL_MAP_H
#define DSY_HAL_MAP_H
#include "stm32h7xx_hal.h"
#include "daisy_core.h"
#include "per_i2c.h"


/** externs of HAL handles...
*/
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;
extern I2C_HandleTypeDef hi2c4;


/** These return a HAL GPIO_TypeDef and HAL GPIO Pin as used in the HAL
from a dsy_gpio_pin input.
*/
GPIO_TypeDef *dsy_hal_map_get_port(dsy_gpio_pin *p);
uint16_t      dsy_hal_map_get_pin(dsy_gpio_pin *p);


/** Returns the I2C_HandleTypeDef for a given dsy_i2c_handle
*/
I2C_HandleTypeDef *dsy_hal_map_get_i2c(dsy_i2c_handle *p);

#endif
