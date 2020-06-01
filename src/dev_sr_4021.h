// # CD4021 Input Shift Register
// ## Description
//
// Device driver for the CD4021
//
// Bit-banged serial shift input.
//
#pragma once
#ifndef DEV_SR_4021_H
#define DEV_SR_4021_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "per_gpio.h"

// ## Defines
// Fixed maximums for parallel/daisychained use
//
// These could be expanded, but haven't been tested beyond this
//
// ~~~~
#define SR_4021_MAX_PARALLEL 2
#define SR_4021_MAX_DAISYCHAIN 1
// ~~~~

// ## Data
// ### Pin Config
// Pins that need to be configured to use
//
// DATA2 only needs to be set if num_parallel is > 1
// ~~~~
enum
{
	DSY_SR_4021_PIN_CS,
	DSY_SR_4021_PIN_CLK,
	DSY_SR_4021_PIN_DATA,
	DSY_SR_4021_PIN_DATA2,//optional
	DSY_SR_4021_PIN_LAST,
};
// ~~~~


// ### sr_4021_handle
// configuration strucutre for 4021
// 
// pin config is used to initialize the dsy_gpio
//
// num_parallel is the number of devices connected 
// that share the same clk/cs, etc. but have independent data
//
// num_daisychained is the number of devices in a daisy-chain configuration
//
// ~~~~
typedef struct
{
	dsy_gpio_pin pin_config[DSY_SR_4021_PIN_LAST];
	uint8_t		 num_parallel, num_daisychained;
	dsy_gpio   cs, clk, data[SR_4021_MAX_PARALLEL];
	uint8_t		 states[8 * SR_4021_MAX_DAISYCHAIN * SR_4021_MAX_PARALLEL];
} dsy_sr_4021_handle;
// ~~~~

// ### Init
// Initialize CD4021 with settinsgs from sr_4021_handle
// ~~~~
void	dsy_sr_4021_init(dsy_sr_4021_handle *sr);
// ~~~~

// ### Update
// Fills internal states with CD4021 data states.
// ~~~~
void dsy_sr_4021_update(dsy_sr_4021_handle *sr); // checks all 8 states per configured device.
// ~~~~
// ### State
// Returns the state of a pin at a given index.
// ~~~~
uint8_t dsy_sr_4021_state(dsy_sr_4021_handle *sr, uint8_t idx);
// ~~~~

#ifdef __cplusplus
}
#endif
#endif
