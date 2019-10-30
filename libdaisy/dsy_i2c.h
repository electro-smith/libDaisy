#ifndef DSY_I2C_H
#define DSY_I2C_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32h7xx_hal.h"
#include "dsy_core_hw.h"

typedef enum
{
	DSY_I2C_PERIPH_1,
	DSY_I2C_PERIPH_2,
	DSY_I2C_PERIPH_3,
	DSY_I2C_PERIPH_4,
} dsy_i2c_periph;
typedef enum
{
	DSY_I2C_PIN_SCL,
	DSY_I2C_PIN_SDA,
	DSY_I2C_PIN_LAST,
} dsy_i2c_pin;
typedef enum
{
	DSY_I2C_SPEED_100KHZ,
	DSY_I2C_SPEED_400KHZ,
	DSY_I2C_SPEED_1MHZ,
	DSY_I2C_SPEED_LAST,
} dsy_i2c_speed;

typedef struct
{
	dsy_i2c_periph periph;
	dsy_gpio_pin  pin_config[DSY_I2C_PIN_LAST];
	dsy_i2c_speed speed;
} dsy_i2c_handle_t;

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

void dsy_i2c_init(dsy_i2c_handle_t *dsy_hi2c);

// Returns a pointer to the HAL I2C Handle, for use in device drivers.
I2C_HandleTypeDef *dsy_i2c_hal_handle(dsy_i2c_handle_t *dsy_hi2c);

#ifdef __cplusplus
}
#endif
#endif /*__ i2c_H */
