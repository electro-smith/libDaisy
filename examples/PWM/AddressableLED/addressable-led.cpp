/** DMA PWM 
 *  
 *  In this example we will use the DMA to generate a PWM sequence
 *  This can be common for things like Digital LEDs (WS8212), motor control, etc.
 * 
 *  For this we're going to use Daisy Seed pin D17 as TIM3 Channel 4 (unless it has issues because of the LED attached..)
 */
#include "daisy_seed.h"

using namespace daisy;

/** Hardware object for communicating with Daisy */
DaisySeed                       hw;
const size_t                    kOutBufferSize = 512;
uint32_t DMA_BUFFER_MEM_SECTION outbuffer[kOutBufferSize];

int main(void)
{
    /** Initialize hardware */
    hw.Init();

    /** Initialize timer */
    TimerHandle::Config tim_cfg;
    TimerHandle         timer;
    tim_cfg.periph = TimerHandle::Config::Peripheral::TIM_3;
    timer.Init(tim_cfg);

    /** Generate period for timer 
     *  This is a marvelously useful little tidbit that should be put into a TimerHandle function or something.
     */
    uint32_t prescaler         = 8;
    uint32_t tickspeed         = (System::GetPClk2Freq() * 2) / prescaler;
    uint32_t target_pulse_freq = 833333; /**< 1.2 microsecond symbol length */
    uint32_t period            = (tickspeed / target_pulse_freq) - 1;
    timer.SetPrescaler(prescaler - 1); /**< ps=0 is divide by 1 and so on.*/
    timer.SetPeriod(period);

    TimChannel::Config chn_cfg;
    chn_cfg.tim  = &timer;
    chn_cfg.chn  = TimChannel::Config::Channel::FOUR;
    chn_cfg.mode = TimChannel::Config::Mode::PWM;
    chn_cfg.pin  = seed::D17;
    TimChannel pwm;
    /** Fill Buffer */
    for(size_t i = 0; i < kOutBufferSize; i++)
    {
        float t      = (float)i / (float)(kOutBufferSize - 1); /**< 0.0->1.0 */
        float ts     = 0.5f + (cos(t * 6.28) * 0.5f);
        outbuffer[i] = (uint32_t)(ts * period);
    }
    /** Initialize PWM */
    pwm.Init(chn_cfg);
    timer.Start();
    pwm.Start();
    System::Delay(1000);
    pwm.StartDma(outbuffer, kOutBufferSize, nullptr);
    bool led_state = true;

    while(1)
    {
        System::Delay(2000);
        hw.SetLed(led_state);
        if(led_state)
            led_state = false;
        else
            led_state = true;
        pwm.StartDma(outbuffer, kOutBufferSize, nullptr);
    }
}
