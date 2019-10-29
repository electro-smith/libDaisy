// This is a Board Specific File
// I don't think it actually belongs in the library.
// Any new piece of hardware can just have their own board file.
// This will allow minor pin changes, etc. not to require changing the library in a million places.
// 
// Some of these peripherals have overlapping pins with different functions.
// Therefore, none of these peripherals will initialize on your own
//   instead the dsy_<periph>_init function will still need to be called. There is a section at
//   the bottom of daisy_seed_init, where you can place a bunch of peripheral init functions,
//   to reduce the portability of code for a specific piece of hardware.

#pragma once
#ifndef DSY_SEED_H
#define DSY_SEED_H

#include "libdaisy.h"


#define SEED_NUM_GPIO 32

// Probably should move this to a dsy_handle.h
// So that it can be used in the other peripheral 
// initializations, etc.  
// (E.g. Audio needs both SAI, and I2C for most devices.)
typedef struct
{
	dsy_gpio_pin generic_pins[SEED_NUM_GPIO];
	dsy_sai_handle_t sai_handle;
	dsy_i2c_handle_t i2c1_handle, i2c2_handle;
} daisy_handle;


// THIS BREAKS WHEN ITS INLINED?
 void daisy_seed_init(daisy_handle *daisy_seed) 
{
	dsy_gpio_pin *pin_group; // Pointer to an arbitrary group of pins.

	// QSPI	

	// FMC

	// SAI - Serial Audio Interface
	daisy_seed->sai_handle.init = DSY_AUDIO_INIT_BOTH;
	// SAI1 - config
	daisy_seed->sai_handle.samplerate[DSY_SAI_1] = DSY_AUDIO_SAMPLERATE_48K;
	daisy_seed->sai_handle.bitdepth[DSY_SAI_1] = DSY_AUDIO_BITDEPTH_16;
	daisy_seed->sai_handle.sync_config[DSY_SAI_1] = DSY_AUDIO_SYNC_MASTER;
	pin_group = daisy_seed->sai_handle.sai1_pin_config;
	pin_group[DSY_SAI_PIN_MCLK].port = DSY_GPIOE;
	pin_group[DSY_SAI_PIN_MCLK].pin = 2;
	pin_group[DSY_SAI_PIN_FS].port = DSY_GPIOE;
	pin_group[DSY_SAI_PIN_FS].pin = 4;
	pin_group[DSY_SAI_PIN_SCK].port = DSY_GPIOE;
	pin_group[DSY_SAI_PIN_SCK].pin = 5;
	pin_group[DSY_SAI_PIN_SIN].port = DSY_GPIOE;
	pin_group[DSY_SAI_PIN_SIN].pin = 6;
	pin_group[DSY_SAI_PIN_SOUT].port = DSY_GPIOE;
	pin_group[DSY_SAI_PIN_SOUT].pin = 3;
	// SAI2 - config
	daisy_seed->sai_handle.samplerate[DSY_SAI_2] = DSY_AUDIO_SAMPLERATE_48K;
	daisy_seed->sai_handle.bitdepth[DSY_SAI_2] = DSY_AUDIO_BITDEPTH_16;
	daisy_seed->sai_handle.sync_config[DSY_SAI_2] = DSY_AUDIO_SYNC_SLAVE;
	pin_group						 = daisy_seed->sai_handle.sai2_pin_config;
	pin_group[DSY_SAI_PIN_MCLK].port = DSY_GPIOA; // unused on daisy patch, but configurable
	pin_group[DSY_SAI_PIN_MCLK].pin = 1; // unused on daisy patch, but configurable
	pin_group[DSY_SAI_PIN_FS].port = DSY_GPIOG; 
	pin_group[DSY_SAI_PIN_FS].pin = 9;
	pin_group[DSY_SAI_PIN_SCK].port = DSY_GPIOA;
	pin_group[DSY_SAI_PIN_SCK].pin = 2;
	pin_group[DSY_SAI_PIN_SIN].port = DSY_GPIOD;
	pin_group[DSY_SAI_PIN_SIN].pin = 11;
	pin_group[DSY_SAI_PIN_SOUT].port = DSY_GPIOA;
	pin_group[DSY_SAI_PIN_SOUT].pin = 0;

	// I2C - Inter-Integrated Circuit
	// TODO: Add Config for I2C3 and I2C4 
	// I2C 1 - (On daisy patch this controls the LED Driver, and the WM8731).
	daisy_seed->i2c1_handle.periph = DSY_I2C_PERIPH_1;
	daisy_seed->i2c1_handle.speed   = DSY_I2C_SPEED_400KHZ;
	pin_group = daisy_seed->i2c1_handle.pin_config;
	pin_group[DSY_I2C_PIN_SCL].port = DSY_GPIOB;
	pin_group[DSY_I2C_PIN_SCL].pin = 8;
	pin_group[DSY_I2C_PIN_SDA].port = DSY_GPIOB;
	pin_group[DSY_I2C_PIN_SDA].pin = 9;
	// I2C 2 - (On daisy patch this controls the on-board WM8731)
	daisy_seed->i2c2_handle.periph = DSY_I2C_PERIPH_2;
	daisy_seed->i2c2_handle.speed   = DSY_I2C_SPEED_400KHZ;
	pin_group = daisy_seed->i2c2_handle.pin_config;
	pin_group[DSY_I2C_PIN_SCL].port = DSY_GPIOH;
	pin_group[DSY_I2C_PIN_SCL].pin = 4;
	pin_group[DSY_I2C_PIN_SDA].port = DSY_GPIOB;
	pin_group[DSY_I2C_PIN_SDA].pin = 11;

	// ADC

	// System Initialization
	uint8_t board = DSY_SYS_BOARD_DAISY_SEED;
	dsy_system_init(board);
	dsy_i2c_init(&daisy_seed->i2c1_handle);
	dsy_i2c_init(&daisy_seed->i2c2_handle);
	dsy_audio_init(&daisy_seed->sai_handle,
				   DSY_AUDIO_DEVICE_WM8731,
				   DSY_AUDIO_DEVICE_WM8731);
}

#endif
