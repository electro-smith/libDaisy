#pragma once
#ifndef DSY_HAL_MAP_H
#define DSY_HAL_MAP_H
#include "stm32h7xx_hal.h"
#include "daisy_core.h"
#include "per_i2c.h"


// externs of HAL handles...
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;
extern I2C_HandleTypeDef hi2c4;


// GPIO Map
GPIO_TypeDef *dsy_hal_map_get_port(dsy_gpio_pin *p);
uint16_t      dsy_hal_map_get_pin(dsy_gpio_pin *p);


// I2C MAp
I2C_HandleTypeDef *dsy_hal_map_get_i2c(dsy_i2c_handle *p);

#endif
