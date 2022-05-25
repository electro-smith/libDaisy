#pragma once
#ifndef DEV_SR_4021_H
#define DEV_SR_4021_H
#include "per/gpio.h"
#include "sys/system.h"

namespace daisy
{
/** @brief Device Driver for CD4021 shift register
 ** @author shensley
 ** @ingroup shiftregister
 **
 ** CD4021B-Q1: CMOS 8-STAGE STATIC SHIFT REGISTER
 ** 
 ** Supply Voltage: 3V to 18V
 ** Clock Freq: 3MHz at 5V (less at 3v3) -> 8.5MHz at 15V
 ** Pin Descriptions:
 ** - Parallel Data[1-8] - 7, 6, 5, 4, 13, 14, 115, 1
 ** - Serial Data        - 11
 ** - Clock              - 10
 ** - P/!S               - 9
 ** - Q[6-8]             - 2, 12, 3
 **
 ** Driver has support for daisy chaining and running up to 2 same-sized 
 ** chains in parallel from a single set of clk/latch pins to reduce 
 ** pin/code overhead when using multiple devices.
 **
 ** When dealing with multiple parallel/daisy-chained devices the 
 ** states of all inputs will be filled in the following order (example uses two chained and two parallel):
 ** data[chain0,parallel0], data[chain1,parallel0], data[chain0,parallel1], data[chain1,parallel1];
 ** 
 ** When combining multiple daisy chained and parallel devices the number of devices chained should match
 ** for each parallel device chain.
 **
 ***/
template <size_t num_daisychained = 1, size_t num_parallel = 1>
class ShiftRegister4021
{
  public:
    /** Configuration Structure for handling the pin setting of the device */
    struct Config
    {
        dsy_gpio_pin clk;   /**< Clock pin to attach to pin 10 of device(s) */
        dsy_gpio_pin latch; /**< Latch pin to attach to pin 9 of device(s) */
        dsy_gpio_pin data[num_parallel]; /**< Data Pin(s) */
    };

    ShiftRegister4021() {}
    ~ShiftRegister4021() {}

    /** Initializes the Device(s) */
    void Init(const Config& cfg)
    {
        config_ = cfg;
        // Init GPIO
        clk_.mode = DSY_GPIO_MODE_OUTPUT_PP;
        clk_.pull = DSY_GPIO_NOPULL;
        clk_.pin  = cfg.clk;
        dsy_gpio_init(&clk_);
        latch_.mode = DSY_GPIO_MODE_OUTPUT_PP;
        latch_.pull = DSY_GPIO_NOPULL;
        latch_.pin  = cfg.latch;
        dsy_gpio_init(&latch_);
        for(size_t i = 0; i < num_parallel; i++)
        {
            data_[i].mode = DSY_GPIO_MODE_INPUT;
            data_[i].pull = DSY_GPIO_NOPULL;
            data_[i].pin  = cfg.data[i];
            dsy_gpio_init(&data_[i]);
        }
        // Init States
        for(size_t i = 0; i < kTotalStates; i++)
        {
            states_[i] = false;
        }
    }

    /** Reads the states of all pins on the connected device(s) */
    void Update()
    {
        dsy_gpio_write(&clk_, 0);
        dsy_gpio_write(&latch_, 1);
        System::DelayTicks(1);
        dsy_gpio_write(&latch_, 0);
        uint32_t idx;
        for(size_t i = 0; i < 8 * num_daisychained; i++)
        {
            dsy_gpio_write(&clk_, 0);
            System::DelayTicks(1);
            for(size_t j = 0; j < num_parallel; j++)
            {
                idx = (8 * num_daisychained - 1) - i;
                idx += (8 * num_daisychained * j);
                states_[idx] = dsy_gpio_read(&data_[j]);
            }
            dsy_gpio_write(&clk_, 1);
            System::DelayTicks(1);
        }
    }

    /** returns the last read state of the input at the index. 
     ** true indicates the pin is held HIGH.
     ** 
     ** See above for the layout of data when using multiple 
     ** devices in series or parallel.
     ***/
    inline bool State(int index) const { return states_[index]; }

    inline const Config& GetConfig() const { return config_; }

  private:
    static constexpr int kTotalStates = 8 * num_daisychained * num_parallel;
    Config               config_;
    bool                 states_[kTotalStates];
    dsy_gpio             clk_;
    dsy_gpio             latch_;
    dsy_gpio             data_[num_parallel];
};

} // namespace daisy

#endif
