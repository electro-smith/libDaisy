/** TCA9534 GPIO expander demo
 *
 * Wiring (Daisy Seed I2C1 defaults):
 * - SCL = D11 (PB8), SDA = D12 (PB9), +3V3, GND
 * - TCA9534 at address 0x20 (A0/A1/A2 = GND), or change kAddress
 * - External pull-ups on SDA/SCL (typically 2.2k–4.7k to 3V3)
 * - External pull-ups on input pins (chip has no internal pull-ups)
 *
 * P0 is driven as an output (blink). P1 is read as an input and printed over USB serial.
 */
#include "daisy_seed.h"

using namespace daisy;

DaisySeed hw;
Tca9534   expander;

// 7-bit I2C address — do not left-shift (libDaisy v8+).
static constexpr uint8_t kAddress = 0x20;

int main(void)
{
    hw.Init(true);
    hw.StartLog(false);

    Tca9534::Config cfg;
    cfg.Defaults();
    cfg.i2c_address = kAddress;
    expander.Init(cfg);

    // P0 = output, P1–P7 = inputs
    expander.SetConfig(0xFE);
    expander.WritePin(0, false);

    uint32_t last_print = System::GetNow();
    bool     led_on     = false;

    while(1)
    {
        const uint32_t now = System::GetNow();
        if(now - last_print > 250)
        {
            last_print = now;
            led_on     = !led_on;
            expander.WritePin(0, led_on);

            uint8_t port = 0;
            bool    p1   = false;
            if(expander.ReadInput(port))
            {
                expander.ReadPin(1, p1);
                hw.PrintLine("OUT0:%d  IN:%02X  P1:%d  ok:%d",
                             led_on ? 1 : 0,
                             port,
                             p1 ? 1 : 0,
                             expander.LastTransferOk() ? 1 : 0);
            }
            else
            {
                hw.PrintLine("TCA9534 read failed (check wiring / address)");
            }
        }
        hw.SetLed((now & 511) < 255);
    }
}
