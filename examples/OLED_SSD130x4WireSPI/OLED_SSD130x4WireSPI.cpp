/** Example of setting up an SSD130x OLED display using 4-wire SPI
 * 
 *  This example will print out the number of seconds since 
 *  bootup on the display.
 * 
 *  The number of seconds will loop back around after about 50 days
 */
#include "daisy_seed.h"

/** Includes the oled display header (not included in Daisy?)*/
#include "dev/oled_ssd130x.h"

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

/** Global Hardware access */
DaisySeed hw;

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    OledDisplay<SSD130x4WireSpi128x64Driver>         display;
    OledDisplay<SSD130x4WireSpi128x64Driver>::Config display_cfg;


    /** SPI Configuration 
     * 
     *  Most default values are still used, but here are a few 
     *  settings that are most likely to be edited for custom hw
     */
    display_cfg.driver_config.transport_config.spi_config.periph
        = SpiHandle::Config::Peripheral::SPI_1;
    display_cfg.driver_config.transport_config.spi_config.baud_prescaler
        = SpiHandle::Config::BaudPrescaler::PS_8;
    /** Set up the SPI Pins for SPI1 */
    display_cfg.driver_config.transport_config.spi_config.pin_config.sclk
        = seed::D8;
    display_cfg.driver_config.transport_config.spi_config.pin_config.miso
        = Pin();
    display_cfg.driver_config.transport_config.spi_config.pin_config.mosi
        = seed::D10;
    display_cfg.driver_config.transport_config.spi_config.pin_config.nss
        = seed::D7;
    /** Command and Reset Pins */
    display_cfg.driver_config.transport_config.pin_config.dc    = seed::D0;
    display_cfg.driver_config.transport_config.pin_config.reset = seed::D32;

    /** Initialize it */
    display.Init(display_cfg);
    /** Set it to black until we start updating. */
    display.Fill(false);
    display.Update();

    /** Infinite Loop */
    while(1)
    {
        System::Delay(1000);

        /** Get the number of seconds since the program started */
        int seconds = System::GetNow() / 1000.0;

        /** Clear the display */
        display.Fill(false);
        /** Set the cursor away from the edge of the screen */
        display.SetCursor(4, 16);
        /** Write the following, starting at the cursor */
        display.WriteString("Time since startup:", Font_6x8, true);
        /** Update the cursor to the next space below the string */
        display.SetCursor(4, 32);

        /** Create a string representation of the */
        FixedCapStr<16> str("");
        str.AppendInt(seconds);
        /** Write the value to the display */
        display.WriteString(str, Font_11x18, true);

        /** And finally push the updated screen contents to the hardware */
        display.Update();
    }
}