/**
 * SR 4021 Example
 * This demonstrates the use of an CD4021 input shift register to read 8 buttons.
 * Some auxillary code is included to debounce, and time the length presses.
 *
 * The AudioCallback is utilized to read the buttons at 1kHz.
 * Values are output via the USB Micro port.
 */
#include "daisy_seed.h"

// Typedef for simplifcation of init/config
using ButtonSr = daisy::ShiftRegister4021<1, 1>;

// Period for printing button data over USB.
static constexpr uint32_t kUsbPrintPeriod = 250;

// Consts for pin locations on the seed -- customize as needed.
// The 4021 operates through "bit-banging" so any GPIO is usable.
static constexpr daisy::Pin kPinSrClk    = daisy::seed::D7;
static constexpr daisy::Pin kPinSrData   = daisy::seed::D19;
static constexpr daisy::Pin kPinSrLatch  = daisy::seed::D21;
static constexpr daisy::Pin kCallbackPin = daisy::seed::D23;

// Global Variables
daisy::DaisySeed hw;
daisy::GPIO      callback_pin;
ButtonSr         btn_shiftreg;
uint8_t          btn_data[8];
uint32_t         btn_rising_edge_time[8];


void AudioCallback(daisy::AudioHandle::InputBuffer  in,
                   daisy::AudioHandle::OutputBuffer out,
                   size_t                           size)
{
    callback_pin.Write(true);
    btn_shiftreg.Update();
    for(size_t i = 0; i < 8; i++)
    {
        auto datum  = btn_shiftreg.State(i);
        btn_data[i] = (btn_data[i] << 1) | (datum ? 0 : 1);
        if(btn_data[i] == 0x7f)
            btn_rising_edge_time[i] = daisy::System::GetNow();
    }
    callback_pin.Write(false);
}

int main(void)
{
    // Initialize the Daisy Seed hardware
    hw.Init(true);

    ButtonSr::Config sr_cfg;
    sr_cfg.clk     = kPinSrClk;
    sr_cfg.latch   = kPinSrLatch;
    sr_cfg.data[0] = kPinSrData;
    btn_shiftreg.Init(sr_cfg);

    callback_pin.Init(kCallbackPin, daisy::GPIO::Mode::OUTPUT);

    hw.StartAudio(AudioCallback);

    hw.StartLog(false);

    uint32_t now, usb_time;
    now = usb_time = daisy::System::GetNow();

    while(1)
    {
        now = daisy::System::GetNow();
        if(now - usb_time > kUsbPrintPeriod)
        {
            // Do the printing of the stuff.
            hw.PrintLine("Button Data:");
            hw.PrintLine("------------");
            for(size_t i = 0; i < 8; i++)
            {
                uint32_t timeheld
                    = btn_data[i] == 0xff ? now - btn_rising_edge_time[i] : 0;
                hw.PrintLine("Button %d: %s -- %d",
                             i,
                             btn_data[i] == 0xff ? "Pressed" : "Released",
                             timeheld);
            }
            usb_time = now;
        }
    }
}
