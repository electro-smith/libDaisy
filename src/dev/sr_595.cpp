#include <algorithm>
#include "dev/sr_595.h"
namespace daisy
{
void ShiftRegister595::Init(Pin *pin_cfg, size_t num_daisy_chained)
{
    // Initialize Pins as outputs
    for(size_t i = 0; i < NUM_PINS; i++)
    {
        pin_[i].Init(pin_cfg[i], GPIO::Mode::OUTPUT);
    }
    std::fill(state_, state_ + kMaxSr595DaisyChain, 0x00);
    num_devices_ = num_daisy_chained;
    // Set to 1 device if out of range.
    if(num_devices_ == 0 || num_devices_ > kMaxSr595DaisyChain)
        num_devices_ = 1;
}
void ShiftRegister595::Set(uint8_t idx, bool state)
{
    uint8_t dev, bit;
    dev = idx / 8;
    bit = idx % 8;
    if(state)
        state_[dev] |= (1 << bit);
    else
        state_[dev] &= ~(1 << bit);
}
void ShiftRegister595::Write()
{
    // This is about 2MHz clock speeds without delays
    // Max Freq is 4-6 MHz at 2V, and 21-31MHz at 4V5.
    pin_[PIN_LATCH].Write(0);
    for(size_t i = 0; i < num_devices_ * 8; i++)
    {
        pin_[PIN_CLK].Write(0);
        pin_[PIN_DATA].Write(state_[((num_devices_ - 1) - (i / 8))]
                             & (1 << (7 - (i % 8))));
        pin_[PIN_CLK].Write(1);
    }
    pin_[PIN_LATCH].Write(1);
}

} // namespace daisy