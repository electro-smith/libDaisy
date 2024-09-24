#include "dev/codec_ak4556.h"
#include "sys/system.h"
#include "per/gpio.h"

namespace daisy
{
void Ak4556::Init(Pin reset_pin)
{
    reset_.Init(reset_pin, GPIO::Mode::OUTPUT);
    reset_.Write(1);
    System::Delay(1);
    reset_.Write(0);
    System::Delay(1);
    reset_.Write(1);
}

void Ak4556::DeInit()
{
    reset_.DeInit();
}


} // namespace daisy
