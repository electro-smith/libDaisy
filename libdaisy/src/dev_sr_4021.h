#pragma once
#ifndef DEV_SR_4021_H
#define DEV_SR_4021_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "per_gpio.h"

// Fixed maximums for parallel/daisychained use
// These could be expanded
#define SR_4021_MAX_PARALLEL 2
#define SR_4021_MAX_DAISYCHAIN 1

enum
{
	DSY_SR_4021_PIN_CS,
	DSY_SR_4021_PIN_CLK,
	DSY_SR_4021_PIN_DATA,
	DSY_SR_4021_PIN_DATA2,//optional
	DSY_SR_4021_PIN_LAST,
};
typedef struct
{
	dsy_gpio_pin pin_config[DSY_SR_4021_PIN_LAST];
	uint8_t		 num_parallel, num_daisychained;
	dsy_gpio   cs, clk, data[SR_4021_MAX_PARALLEL];
	uint8_t		 states[8 * SR_4021_MAX_DAISYCHAIN * SR_4021_MAX_PARALLEL];
} dsy_sr_4021_handle;

void	dsy_sr_4021_init(dsy_sr_4021_handle *sr);
void dsy_sr_4021_update(dsy_sr_4021_handle *sr); // checks all 8 states per configured device.
uint8_t dsy_sr_4021_state(dsy_sr_4021_handle *sr, uint8_t idx);

#ifdef __cplusplus
}
#endif
#endif
