#include "daisy_seed.h"

using namespace daisy;

DaisySeed hw;

int main(void)
{
	// Initialize the hardware
	hw.Init();

	// Handle we'll use to interact with SPI
	SpiHandle spi_handle;

	// Structure to configure the SPI handle
	SpiHandle::Config spi_conf;

	spi_conf.mode = SpiHandle::Config::Mode::MASTER; // we're in charge

	spi_conf.periph = SpiHandle::Config::Peripheral::SPI_1; // Use the SPI_1 Peripheral

	// Pins to use. These must be available on the selected peripheral
	spi_conf.pin_config.sclk = Pin(PORTG, 11); // Use pin PG11 as SCLK
	spi_conf.pin_config.miso = Pin(); // We won't need this
	spi_conf.pin_config.mosi = Pin(PORTB, 5); // Use PB5 as MOSI
	spi_conf.pin_config.nss = Pin(PORTG, 10); // use PG10 as NSS

	// data will flow from master to slave over just the MOSI line
	spi_conf.direction = SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;

	// The master will output on the NSS line
	spi_conf.nss = SpiHandle::Config::NSS::HARD_OUTPUT;

	// Initialize the SPI Handle
	spi_handle.Init(spi_conf);

	// loop forever
	while(1) 
	{
		// put these four bytes in a buffer
		uint8_t buffer[4] = {0, 1, 2, 3};

		// transmit those 4 bytes
		spi_handle.BlockingTransmit(buffer, 4);

		// wait 500 ms
		System::Delay(500);
	}
}
