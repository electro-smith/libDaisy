/* TODO:
- Add configurable tick frequency -- for now its set to the APB1 Max Freq (200MHz)
- Add ability to generate periodic callback functions
*/
#pragma once
#ifndef DSY_TIM_H
#define DSY_TIM_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

    /** @addtogroup other
    @{
    */


    /** General purpose timer for delays and general timing. */

    /** initializes the TIM2 peripheral with maximum counter autoreload, and no prescalers. */
    void dsy_tim_init();

    /** Starts the timer ticking. */
    void dsy_tim_start();

    /** These functions are specific to the actual clock ticks at the timer frequency which is currently fixed at 200MHz
        \return a number 0x00000000-0xffffffff of the current tick
    */
    uint32_t dsy_tim_get_tick();

    /** 
    blocking delay of cnt timer ticks. 
    \param cnt Number of ticks
     */
    void dsy_tim_delay_tick(uint32_t cnt);

    /** These functions are converted to use milliseconds as their time base.
    \return the number of milliseconds through the timer period.
    */
    uint32_t dsy_tim_get_ms();

    /** blocking delay of cnt milliseconds.
    \param cnt Delay time in ms
    */
    void dsy_tim_delay_ms(uint32_t cnt);

    /** These functions are converted to use microseconds as their time base.
    \return the number of microseconds through the timer period.
    */
    uint32_t dsy_tim_get_us();

    /** blocking delay of cnt microseconds. 
    \param cnt Delay time in us
     */
    void dsy_tim_delay_us(uint32_t cnt);
#ifdef __cplusplus
}
#endif
#endif
/** @} */
