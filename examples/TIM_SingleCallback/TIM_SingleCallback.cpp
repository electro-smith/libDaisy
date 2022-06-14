/** TIM Single Callback 
 *  Example of setting up and using a TIM peripheral
 *  to generate periodic callbacks
 * 
 *  To demonstrate, the LED will be toggled from the callback
 */
#include "daisy_seed.h"

using namespace daisy;

/** Create Daisy Seed object */
DaisySeed hw;

/** This is our Timer-generated callback
 * 
 *  Once the timer has been configured, and started, this function will be called
 *  at the end of each timer period.
 */
void Callback(void* data)
{
    /** Use system time to blink LED once per second (1023ms) */
    bool led_state = (System::GetNow() & 1023) > 511;
    /** Set LED */
    hw.SetLed(led_state);
}

int main(void)
{
    /** Initialize Daisy Seed */
    hw.Init();

    /** Create Handle and config 
     *  We'll use TIM5 here, but TIM3, and TIM4 are also available
     *  At this time, TIM2 is used by the System class for sub-millisecond time/delay functions 
     */
    TimerHandle         tim5;
    TimerHandle::Config tim_cfg;

    /** TIM5 with IRQ enabled */
    tim_cfg.periph     = TimerHandle::Config::Peripheral::TIM_5;
    tim_cfg.enable_irq = true;

    /** Configure frequency (30Hz) */
    auto tim_target_freq = 30;
    auto tim_base_freq   = System::GetPClk2Freq();
    tim_cfg.period       = tim_base_freq / tim_target_freq;

    /** Initialize timer */
    tim5.Init(tim_cfg);
    tim5.SetCallback(Callback);

    /** Start the timer, and generate callbacks at the end of each period */
    tim5.Start();

    /** Do nothing here, and rely on callback to toggle LED */
    while(1) {}
}
