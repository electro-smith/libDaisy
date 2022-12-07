/** Basic PWM 
 *  
 *  In this example, we will configure a single GPIO
 *  to output PWM using the TIM hardware built into the daisy.
 * 
 *  For this we're going to use Daisy Seed pin D17 as TIM3 Channel 4
 */
#include "daisy_seed.h"

using namespace daisy;

/** Hardware object for communicating with Daisy */
DaisySeed   hw;
TimerHandle tim;

int main(void)
{
    /** Initialize hardware */
    hw.Init();

    /** Set up TIM for PWM 
     *  800kHz clock rate, w/ 8-bit resolution
     *  = about 3.125kHz refresh rate
     * 
     *  Typical uses could work with much slower timers, or higher resolution
     */
    TimerHandle::Config tim_cfg;
    tim_cfg.periph = TimerHandle::Config::Peripheral::TIM_3;
    tim_cfg.period = 65536; /**< Not correct for 800kHz */
    tim.Init(tim_cfg);
    tim.SetPeriod(65536);
    tim.SetPrescaler(1);

    TimChannel::Config chn_cfg;
    chn_cfg.tim  = &tim;
    chn_cfg.chn  = TimChannel::Config::Channel::THREE;
    chn_cfg.mode = TimChannel::Config::Mode::PWM_GENERATION;
    chn_cfg.pin  = seed::D17;
    TimChannel pwm;
    /** Initialize PWM */
    pwm.Init(chn_cfg);
    tim.SetPeriod(65536);
    tim.SetPrescaler(1);
    tim.Start();
    pwm.Start();

    /** Step through some brightness values */
    int vals[8];
    for(int i = 0; i < 8; i++)
        vals[i] = i * (tim_cfg.period / 8);

    while(1)
    {
        for(int i = 0; i < 8; i++)
        {
            pwm.SetPwm(vals[i]);
            System::Delay(250);
        }
    }
}
