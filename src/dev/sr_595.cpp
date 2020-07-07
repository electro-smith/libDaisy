#include <algorithm>
#include "dev/sr_595.h"
void ShiftRegister595::Init(dsy_gpio_pin *pin_cfg, size_t num_daisy_chained)
{
    // Initialize Pins as outputs
    for(size_t i = 0; i < NUM_PINS; i++)
    {
        pin_[i].pin  = pin_cfg[i];
        pin_[i].mode = DSY_GPIO_MODE_OUTPUT_PP;
        pin_[i].pull = DSY_GPIO_NOPULL;
        dsy_gpio_init(&pin_[i]);
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
    dsy_gpio_write(&pin_[PIN_LATCH], 0);
    for(size_t i = 0; i < num_devices_ * 8; i++)
    {
        dsy_gpio_write(&pin_[PIN_CLK], 0);
        dsy_gpio_write(&pin_[PIN_DATA],
                       state_[((num_devices_ - 1) - (i / 8))]
                           & (1 << (7 - (i % 8))));
        dsy_gpio_write(&pin_[PIN_CLK], 1);
    }
    dsy_gpio_write(&pin_[PIN_LATCH], 1);
}
