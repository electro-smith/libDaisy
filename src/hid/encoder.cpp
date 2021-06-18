#include "hid/encoder.h"

using namespace daisy;

void Encoder::Init(Pin a, Pin b, Pin click, float update_rate)
{
    GPIO::Config conf;

    // Init GPIO for A, and B
    conf.pin  = a;
    conf.pull = GPIO::Config::Pull::PULLUP;
    hw_a_.Init(conf);
    conf.pin = b;
    hw_b_.Init(conf);
    // Default Initialization for Switch
    sw_.Init(click, update_rate);
    // Set initial states, etc.
    inc_ = 0;
    a_ = b_ = 0xff;
}

void Encoder::Debounce()
{
    // Shift Button states to debounce
    a_ = (a_ << 1) | hw_a_.Read();
    b_ = (b_ << 1) | hw_b_.Read();
    // Debounce built-in switch
    sw_.Debounce();
    // infer increment direction
    inc_ = 0; // reset inc_ first
    if((a_ & 0x03) == 0x02 && (b_ & 0x03) == 0x00)
    {
        inc_ = 1;
    }
    else if((b_ & 0x03) == 0x02 && (a_ & 0x03) == 0x00)
    {
        inc_ = -1;
    }
}
