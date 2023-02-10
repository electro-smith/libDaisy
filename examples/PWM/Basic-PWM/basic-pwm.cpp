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
DaisySeed hw;

int main(void)
{
    /** Initialize hardware */
    hw.Init();

    /** Configure frequency (12kHz) */
    auto tim_target_freq = 12000;
    auto tim_base_freq   = System::GetPClk2Freq();

    TimerHandle::Config tim_cfg;
    TimerHandle         timer;
    tim_cfg.periph     = TimerHandle::Config::Peripheral::TIM_3;
    tim_cfg.period     = tim_base_freq / tim_target_freq;
    timer.Init(tim_cfg);

    TimChannel::Config chn_cfg;
    chn_cfg.tim  = &timer;
    chn_cfg.chn  = TimChannel::Config::Channel::FOUR;
    chn_cfg.mode = TimChannel::Config::Mode::PWM;
    chn_cfg.pin  = seed::D17;
    TimChannel pwm;
    /** Initialize PWM */
    pwm.Init(chn_cfg);
    timer.Start();
    pwm.Start();

    /** Step through some brightness values */
    int vals[32];
    for(int i = 0; i < 32; i++)
        vals[i] = i * (tim_cfg.period / 32);

    while(1)
    {
        /* ~30Hz animation of our little 32-frame ramp wave. */
        for(int i = 0; i < 32; i++)
        {
            pwm.SetPwm(vals[i]);
            System::Delay(33);
        }
    }
}
