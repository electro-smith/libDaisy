/** Reads data from an MPR121 capacitive touch sensor
 *
 *  The results are printed over serial.
 *  Each channel will output either "H" or "L" for whether it's touched,
 *  and then display in paranthesis the "filtered value" / "baseline value"
*/
#include "daisy_seed.h"
#include <cmath>

using namespace daisy;

DaisySeed hw;
Mpr121I2C sensor;

int main(void)
{
    hw.Init(true);

    System::Delay(100);

    // Initialize MPR121 w/ default settings
    Mpr121I2C::Config sensor_cfg;
    sensor.Init(sensor_cfg);

    hw.StartLog(false);
    uint32_t print_time = System::GetNow();

    while(1) {
        auto now = System::GetNow();

        if (now - print_time > 100) {
            // Acquire new data
            uint16_t touched = sensor.Touched();
            uint16_t data_base[12];
            uint16_t data_filt[12];
            for (int i = 0; i < 12; i++) {
                data_base[i] = sensor.BaselineData(i);
                data_filt[i] = sensor.FilteredData(i);
            }

            // Print the data
            hw.PrintLine("\nStates:");
            for (int i = 0; i < 12; i++) {
                if (i % 4 == 0)
                    hw.Print("\n");
                bool sta = (touched & (1 << i)) > 0;
                hw.Print("%d: %s (%d / %d)\t", i, sta ? "H" : "L", data_filt[i], data_base[i]);
            }
            hw.Print("\n");
            print_time = System::GetNow();
        }

        // Blink
        hw.SetLed((now & 511) < 255);
    }
}