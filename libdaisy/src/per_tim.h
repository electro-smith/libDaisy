// # per_tim
// General purpose timer for delays and general timing.
//
// TODO:
//
// - Add configurable tick frequency -- for now its set to the APB1 Max Freq (200MHz)
// - Add ability to generate periodic callback functions
#pragma once
#ifndef DSY_TIM_H
#define DSY_TIM_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
// ### init
// initializes the TIM2 peripheral with maximum counter autoreload, and no prescalers.
// ~~~~
void dsy_tim_init();
// ~~~~
// ### start
// Starts the timer ticking.
// ~~~~
void dsy_tim_start();
// ~~~~
// ## tick
// These functions are specific to the actual clock ticks at the timer frequency 
// which is currently fixed at 200MHz
// ### get_tick
// Returns a number 0x00000000-0xffffffff of the current tick
// ~~~~
uint32_t dsy_tim_get_tick();
// ~~~~
// ### delay_tick
// blocking delay of cnt timer ticks.
// ~~~~
void	 dsy_tim_delay_tick(uint32_t cnt);
// ~~~~
// ## ms
// These functions are converted to use milliseconds as their time base.
// ### get_ms
// returns the number of milliseconds through the timer period.
// ~~~~
uint32_t dsy_tim_get_ms();
// ~~~~
// ### delay_ms
// blocking delay of cnt milliseconds.
// ~~~~
void	 dsy_tim_delay_ms(uint32_t cnt);
// ~~~~
// ## us
// These functions are converted to use microseconds as their time base.
// ### get_us
// returns the number of microseconds through the timer period.
// ~~~~
uint32_t dsy_tim_get_us();
// ~~~~
// ### delay_us
// blocking delay of cnt microseconds.
// ~~~~
void	 dsy_tim_delay_us(uint32_t cnt);
// ~~~~
#ifdef __cplusplus
}
#endif
#endif