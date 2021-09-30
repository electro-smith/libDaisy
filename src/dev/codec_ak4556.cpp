#include "dev/codec_ak4556.h"
#include "sys/system.h"
#include "per/gpio.h"

namespace daisy
{
void Ak4556::Init(Pin reset_pin)
{
    GPIO         reset;
    GPIO::Config gpio_conf;

    gpio_conf.pin  = reset_pin;
    gpio_conf.mode = GPIO::Config::Mode::OUTPUT_PP;
    gpio_conf.pull = GPIO::Config::Pull::NOPULL;
    reset.Init(gpio_conf);
    reset.Write(1);
    System::Delay(1);
    reset.Write(0);
    System::Delay(1);
    reset.Write(1);
}

void Ak4556::DeInit(Pin reset_pin)
{
    GPIO reset;
    GPIO::Config gpio_conf;
    gpio_conf.pin = reset_pin;
    reset.DeInit();
}


} // namespace daisy
