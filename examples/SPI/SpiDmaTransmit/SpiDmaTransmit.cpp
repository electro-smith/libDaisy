/**
 * @author beserge
 * @brief 
 * @date 2022-05-19
 * 
 * SPI DMA Transmit
 * Shows how to transmit some data over SPI on the Daisy Patch SM using DMA.
 * (DMA = Direct Memory Access). This method is non-blocking, meaning the hardware will
 * handle the transfer in the background, and the code is free to continue on.
 */

#include "daisy_patch_sm.h"

using namespace daisy;
using namespace patch_sm;

DaisyPatchSM hw;

// put these four bytes in a buffer. It has to be in a special memory section to be used with DMA
uint8_t DMA_BUFFER_MEM_SECTION buffer[4] = {0, 1, 2, 3};

int main(void)
{
	// Initialize the hardware
	hw.Init();

	// Handle we'll use to interact with SPI
	SpiHandle spi_handle;

	// Structure to configure the SPI handle
	SpiHandle::Config spi_conf;

	spi_conf.mode = SpiHandle::Config::Mode::MASTER; // we're in charge

	spi_conf.periph = SpiHandle::Config::Peripheral::SPI_2; // Use the SPI_2 Peripheral

	// Pins to use. These must be available on the selected peripheral
	spi_conf.pin_config.sclk = DaisyPatchSM::D10; // Use pin D10 as SCLK
	spi_conf.pin_config.miso = Pin(); // We won't need this
	spi_conf.pin_config.mosi = DaisyPatchSM::D9; // Use D9 as MOSI
	spi_conf.pin_config.nss = DaisyPatchSM::D1; // use D1 as NSS

	// data will flow from master to slave over just the MOSI line
	spi_conf.direction = SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;

	// The master will output on the NSS line
	spi_conf.nss = SpiHandle::Config::NSS::HARD_OUTPUT;

	// Initialize the SPI Handle
	spi_handle.Init(spi_conf);

	// loop forever
	while(1) 
	{
		// Use DMA to Transmit those 4 bytes. We don't have to wait for this to complete, we could go on and do other things here
		// We won't use the callback or callback data features either, so we'll set them to NULL
		spi_handle.DmaTransmit(buffer, 4, NULL, NULL, NULL);

		// wait 500 ms
		System::Delay(500);
	}
}
