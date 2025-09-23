// Hardware PWM demo
//
// Demonstrates how to use a PWMHandle to output hardware-generated PWM.
// No special setup is required, as the built-in LED on the Seed is used for display.
//
#include "daisy_seed.h"
#include <cmath>

using namespace daisy;
using namespace daisy::seed;

DaisySeed hw;

const float TWO_PI = 6.2831853072f;

int main(void)
{
    // Initialize the Daisy Seed hardware
    hw.Init();
    hw.StartLog(false);

    // First, we'll create a PWMHandle that enables PWM mode on a given timer. Each handle provides up to 4 output channels.
    PWMHandle pwm_tim3;

    {
        // Configure the PWM peripheral
        PWMHandle::Config pwm_config;
        // We'll use TIM3 for PWM output.
        pwm_config.periph = PWMHandle::Config::Peripheral::TIM_3;
        // TIM3 is a 16-bit timer, so the max period is 0xffff. We'll use 0xff to give a higher-frequency PWM signal at the expense of lower precision.
        pwm_config.period = 0xff;

        // Initialize
        if(pwm_tim3.Init(pwm_config) != PWMHandle::Result::OK)
        {
            hw.PrintLine("Could not initialize PWM handle");
        }

        // You can also create the config inline when initializing:
        // pwm_tim3.Init({PWMHandle::Config::Peripheral::TIM_3});
    }

    {
        // Next, configure an individual channel. We'll use Channel 2, which can connect to the Seed's internal LED
        PWMHandle::Channel::Config channel_config;
        // Each timer and channel supports a different set of pins. If no pin is selected, the default for that channel will be selected.
        channel_config.pin = {PORTC, 7};
        // Polarity can be changed for individual channels
        channel_config.polarity = PWMHandle::Channel::Config::Polarity::HIGH;

        // Initialize
        if(pwm_tim3.Channel2().Init(channel_config) != PWMHandle::Result::OK)
        {
            hw.PrintLine("Could not initialize PWM channel");
        }

        // Like before, you can also create the config inline, or leave it blank to use defaults.
        // pwm_tim3.Channel2().Init();

        // Each PWMHandle supports up to 4 channels at once.
    }

    // Instead of calling pwm_tim3.Channel2() every time, you can also store a reference. Note that this reference is valid even if taken before initialization,
    // but it must be initialized after the PWMHandle and before calling Set().
    auto& led_pwm = pwm_tim3.Channel2();

    float phase = 0.0f;
    while(1)
    {
        // Generate a 1 Hz pulse
        float brightness = std::cos(TWO_PI * phase) * 0.5f + 0.5f;

        // Set the PWM channel duty cycle. We also apply a cubic gamma correction factor here to linearize the LED's brightness
        // When calling this method with a float, the value is normalized to [0, period]
        led_pwm.Set(brightness * brightness * brightness);

        // You can also call SetRaw to directly set the duty cycle.
        // led_pwm.SetRaw(0x7f);

        phase += 0.01f;
        if(phase > 1.0f)
            phase -= 1.0f;

        hw.DelayMs(10);
    }
}
