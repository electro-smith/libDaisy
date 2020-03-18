// # PER_I2C
// Driver for controlling I2C devices 
// 
// TODO:
// 
// - Add DMA support
// - Add timing calc based on current clock source freq.
// - Add discrete rx/tx functions (currently other drivers still need to call ST HAL functions).
//
// Errata:
// 
// - 1MHZ (FastMode+) is currently only 886kHZ
// 
#ifndef DSY_I2C_H
#define DSY_I2C_H
#ifdef __cplusplus
extern "C"
{
#endif

//#include "stm32h7xx_hal.h"
#include "daisy_core.h"

// ## Types
// ### dsy_i2c_periph
// Specifices the internal peripheral to use (these are mapped to different pins on the hardware).
//
// ~~~~
typedef enum
{
	DSY_I2C_PERIPH_1,
	DSY_I2C_PERIPH_2,
	DSY_I2C_PERIPH_3,
	DSY_I2C_PERIPH_4,
} dsy_i2c_periph;
// ~~~~
// ### dsy_i2c_pin
// List of pins associated with the peripheral. These must be set in the handle's pin_config.
//
// ~~~~
typedef enum
{
	DSY_I2C_PIN_SCL,
	DSY_I2C_PIN_SDA,
	DSY_I2C_PIN_LAST,
} dsy_i2c_pin;
// ~~~~
// ### dsy_i2c_speed
// Rate at which the clock/data will be sent/received. The device being used will have maximum speeds.
// 
// **1MHZ Mode is currently 886kHz**
// ~~~~
typedef enum
{
	DSY_I2C_SPEED_100KHZ,
	DSY_I2C_SPEED_400KHZ,
	DSY_I2C_SPEED_1MHZ,
	DSY_I2C_SPEED_LAST,
} dsy_i2c_speed;
// ~~~~

// ### dsy_i2c_handle
// this object will be used to initialize the I2C interface, 
// and can be passed to dev_ drivers that require I2C.
// ~~~~
typedef struct
{
	dsy_i2c_periph periph;
	dsy_gpio_pin  pin_config[DSY_I2C_PIN_LAST];
	dsy_i2c_speed speed;
} dsy_i2c_handle;
// ~~~~

//extern I2C_HandleTypeDef hi2c1;
//extern I2C_HandleTypeDef hi2c2;

// ## Functions
// ### dsy_i2c_init 
// initializes an I2C peripheral with the data given from the handle.
// 
// Requires a dsy_i2c_handle object to initialize.
// 
// ~~~~
void dsy_i2c_init(dsy_i2c_handle *dsy_hi2c);
// ~~~~

//// ### dsy_i2c_hal_handle
////
//// Returns a pointer to the HAL I2C Handle, for use in device drivers.
//// 
//// ~~~~
//I2C_HandleTypeDef *dsy_i2c_hal_handle(dsy_i2c_handle *dsy_hi2c);
//// ~~~~

#ifdef __cplusplus
}
#endif
#endif /*__ i2c_H */
