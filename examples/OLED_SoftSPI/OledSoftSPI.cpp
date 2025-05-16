/**
 * @author eh2k
 * @brief 
 * @date 2022-12-01
 * 
 * OLED using software spi
 * Shows how to setup an OLED display on the Daisy Patch SM using software spi.
 * Currently there seems to be some kind of problems with the hardware spi.
 * Here is the possibility to run the oled via software spi. Should work on any gpio pins without interrupt blocking.
 */


#include "daisy_patch_sm.h"
#include "dev/oled_ssd130x.h"

using namespace daisy;
using namespace patch_sm;

DaisyPatchSM hw;

using namespace daisy;
using namespace daisy::patch_sm;

using MyDisplay = OledDisplay<SSD130x4WireSoftSpi128x64Driver>;
MyDisplay display;

int main(void)
{
    // Initialize the hardware
    hw.Init();

    MyDisplay::Config display_config;
    display_config.driver_config.transport_config.pin_config.sclk
        = DaisyPatchSM::D10;
    display_config.driver_config.transport_config.pin_config.sclk_delay = 0;
    display_config.driver_config.transport_config.pin_config.mosi
        = DaisyPatchSM::D9;
    display_config.driver_config.transport_config.pin_config.dc
        = DaisyPatchSM::D2;
    display_config.driver_config.transport_config.pin_config.reset
        = DaisyPatchSM::D3;
    display.Init(display_config);

    char tmp[64];

    // loop forever
    while(1)
    {
        display.Fill(false);
        display.SetCursor(0, 0);
        sprintf(tmp, "%d", System::GetUs());
        display.WriteString(tmp, Font_6x8, true);
        display.Update();
    }
}
