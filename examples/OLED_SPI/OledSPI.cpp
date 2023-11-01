/**
 * @author eh2k
 * @brief 
 * @date 2023-01-08
 * 
 * OLED using hardware spi
 * Shows how to setup an OLED display on the Daisy Patch SM using hardware spi.
 */

#include "daisy_patch_sm.h"
#include "dev/oled_ssd130x.h"

using namespace daisy;
using namespace patch_sm;

DaisyPatchSM hw;

using namespace daisy;
using namespace daisy::patch_sm;

using MyDisplay = OledDisplay<SSD130x4WireSpi128x64Driver>;
MyDisplay display;

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    //
}

int main(void)
{
    // Initialize the hardware
    hw.Init();
    hw.StartAudio(AudioCallback);

    MyDisplay::Config display_config;

    SpiHandle::Config& spi_conf = display_config.driver_config.transport_config.spi_config;

    spi_conf.mode = SpiHandle::Config::Mode::MASTER;             // we're in charge
    spi_conf.periph = SpiHandle::Config::Peripheral::SPI_2;      // Use the SPI_2 Peripheral
    spi_conf.direction = SpiHandle::Config::Direction::ONE_LINE; // TWO_LINES_TX_ONLY;

    spi_conf.datasize = 8;
    spi_conf.clock_polarity = SpiHandle::Config::ClockPolarity::LOW;
    spi_conf.clock_phase = SpiHandle::Config::ClockPhase::ONE_EDGE;
    // spi_conf.nss = SpiHandle::Config::NSS::HARD_OUTPUT;
    spi_conf.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_128;

    // Pins to use. These must be available on the selected peripheral
    spi_conf.pin_config.sclk = DaisyPatchSM::D10; // Use pin D10 as SCLK
    spi_conf.pin_config.miso = Pin();             // We won't need this
    spi_conf.pin_config.mosi = DaisyPatchSM::D9;  // Use D9 as MOSI
    spi_conf.pin_config.nss = Pin();              // DaisyPatchSM::D1;   // use D1 as NSS

    // data will flow from master to slave over just the MOSI line

    // The master will output on the NSS line
    spi_conf.nss = SpiHandle::Config::NSS::SOFT;

    display_config.driver_config.transport_config.pin_config.dc
        = DaisyPatchSM::D2;
    display_config.driver_config.transport_config.pin_config.reset
        = DaisyPatchSM::D3;
    display.Init(display_config);

    char tmp[64];

    // loop forever
    while(1)
    {
        ScopedIrqBlocker _;

        display.Fill(false);
        display.SetCursor(0, 0);
        sprintf(tmp, "%d", System::GetUs());
        display.WriteString(tmp, Font_6x8, true);
        display.Update();
    }
}
