#pragma once
#ifndef DSY_TIM_H
#define DSY_TIM_H

#ifdef __cplusplus
#include <cstdint>

namespace daisy
{
/** Hardare timer peripheral support.
 ** 
 ** Supports general-function TIM peripherals:
 ** - TIM2, TIM3, TIM4, TIM5
 **
 ** DaisySeed, and many internal peripherals utilize TIM2 for timing/delay purposes.
 ** It is configured to be at the maximum frequency: typically 200MHz or 240MHz (boost) 
 ** for measuring/delaying for very short periods. 
 **
 ** TODO:
 ** - Dispatch periodic callback(s)
 ** - Other General purpose timers
 ** - Non-internal clock sources
 ** - Use of the four-tim channels per tim
 **     - PWM, etc.
 **     - InputCapture/OutputCompare, etc.
 ** - HRTIM
 ** - Advanced timers (TIM1/TIM8)
 ** */
class TimerHandle
{
  public:
    struct Config
    {
        /** Hardwaare Timer to configure, and use. */
        enum class Peripheral
        {
            TIM_2 = 0, /**< 32-bit counter */
            TIM_3,     /**< 16-bit counter */
            TIM_4,     /**< 16-bit counter */
            TIM_5,     /**< 32-bit counter*/
        };

        /** Direction of the auto-reload counter. 
         ** TODO: Add support for the various  
         ** versions of Up/Down counters.
         ** */
        enum class CounterDir
        {
            UP = 0,
            DOWN,
        };

        Peripheral periph;
        CounterDir dir;
    };

    /** Return values for TIM funcitons. */
    enum class Result
    {
        OK,
        ERR,
    };

    TimerHandle() : pimpl_(nullptr) {}
    TimerHandle(const TimerHandle& other) = default;
    TimerHandle& operator=(const TimerHandle& other) = default;
    ~TimerHandle() {}

    /** Initializes the timer according to the configuration */
    Result Init(const Config& config);

    /** Returns a const reference to the Config struct */
    const Config& GetConfig() const;

    /** Sets the period of the Timer.
     ** This is the number of ticks it takes before it wraps back around.
     ** For self-managed timing, this can be left at the default. (0xffff for 16-bit
     ** and 0xffffffff for 32-bit timers). 
     ** This can be changed "on-the-fly" 
     ** */
    Result SetPeriod(uint32_t ticks);

    /** Sets the Prescalar applied to the TIM peripheral. 
     ** This can be any number up to 0xffff 
     ** This will adjust the rate of ticks:
     ** Calculated as APBN_Freq / prescalar per tick
     ** where APBN is APB1 for Most general purpose timers,
     ** and APB2 for HRTIM,a nd the advanced timers. 
     ** This can be changed "on-the-fly" 
     ** */
    Result SetPrescaler(uint32_t val);

    /** Starts the TIM peripheral specified by Config */
    Result Start();

    /** Stops the TIM peripheral specified by Config */
    Result Stop();

    /** Returns the frequency of each tick of the timer in Hz */
    uint32_t GetFreq();

    /** Returns the number of counter position. 
     ** This increments according to Config::CounterDir, 
     ** and wraps around at the specified period (maxing out 
     ** at 2^16 or 2^32 depending on the chosen TIM peripheral. */
    uint32_t GetTick();

    /** Returns the ticks scaled as milliseconds 
     **
     ** Use care when using for measurements and ensure that 
     ** the TIM period can handle the maximum desired measurement.
     ***/
    uint32_t GetMs();

    /** Returns the ticks scaled as microseconds 
     **
     ** Use care when using for measurements and ensure that 
     ** the TIM period can handle the maximum desired measurement.
     ***/
    uint32_t GetUs();

    /** Returns the ticks scaled as nanoseconds 
     **
     ** Use care when using for measurements and ensure that 
     ** the TIM period can handle the maximum desired measurement.
     ***/
    uint32_t GetNs();

    /** Stay within this function for del ticks */
    void DelayTick(uint32_t del);

    /** Stay within this function for del milliseconds */
    void DelayMs(uint32_t del);

    /** Stay within this function for del microseconds */
    void DelayUs(uint32_t del);

    /** Stay within this function for del nanoseconds */
    void DelayNs(uint32_t del);

    class Impl;

  private:
    Impl* pimpl_;
};

} // namespace daisy

#endif

#if 0

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
#endif
/** @} */
