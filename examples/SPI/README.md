# SPI

SPI stands for Serial Peripheral Interface, and is a way for devices to talk to one another.
The protocol is used with one device as the main device or the leader.
All other devices are followers.

There are four pins generally used:
- MOSI: Main Out Serial In
- MISO: Main In Serial Out
- SCLK: Serial Clock
- NSS: Serial Select (Chip Select)

## How to Use SPI

First create your SpiHandle object and a Config struct to configure it.
Set any configurations you want to change in the config object, then initialize the spi peripheral with it.

A typical Daisy Seed configuration might look like this

```cpp
// SpiHandle object and Spi Configuration object
SpiHandle spi_handle;
SpiHandle::Config spi_conf;

// Set some configurations
spi_conf.periph = SpiHandle::Peripheral::SPI_1;
spi_conf.mode = SpiHandle::Mode::MASTER;
spi_conf.direction = SpiHandle::Direction::TWO_LINES;
spi_conf.nss = SpiHandle::NSS::NSS_HARD_OUTPUT;
spi_conf.pin_config.sclk = Pin(PORTG, 11);
spi_conf.pin_config.miso = Pin(PORTB, 4);
spi_conf.pin_config.mosi = Pin(PORTB, 5);
spi_conf.pin_config.nss = Pin(PORTG, 10);

// Initialize the handle using our configuration
spi_handle.Init(spi_conf);
```

### Configuration

The default configuration sets the common defaults for SPI peripherals. However some peripherals may require changing these. Some other configurations you'll have to set every time you use SPI.

`spi_conf.peripheral`
The peripheral to be used. Must be specified by the user. For example `SpiHandle::Config::Peripheral::SPI_2`.
The Daisy has 6 different SPI peripherals available, each of which uses different pins. Refer to the pinout diagram for more details.

`spi_conf.mode`
Master or slave. The main device is in charge of the bus (sets the clock, and NSS), the other devices follow along. Most of the time the Daisy will be in charge of the bus.
- `SpiHandle::Config::Mode::MASTER`. The Daisy runs the bus.
- `SpiHandle::Config::Mode::SLAVE`. The Daisy follows another device.

`spi_conf.pin_config`
The pins to be used by this SPI peripheral. These will have to match the peripheral you choose. Must be specified by the user.

- `spi_conf.pin_config.miso`: Main In Serial Out: The main device reads from this pin, and the others write to it.
- `spi_conf.pin_config.mosi`: Main Out Serial In: The main device writes to this pin, and the others read from it.
- `spi_conf.pin_config.sclk`: Serial Clock. The main device outputs a clock signal on this pin.
- `spi_conf.pin_config.nss`: Serial Select. The main device uses this to indicate data is being sent. Usually active low.

If you're not using a pin (e.g. software NSS, or simplex communication) you can set it to `Pin()`.

`spi_conf.direction`
Which direction data will travel. Must be specified by the user.

- `SpiHandle::Config::Direction::TWO_LINES`. Data goes both ways. Each line only goes one direction. i.e. MOSI -> MISO and MISO <- MOSI. This is full duplex. 
- `SpiHandle::Config::Direction::TWO_LINES_TX_ONLY`. The Daisy will only send data. If Daisy's the main device: MOSI->. If the Daisy's a follower MISO->. This is simplex TX.
- `SpiHandle::Config::Direction::TWO_LINES_RX_ONLY`. The Daisy will only read data. If Daisy;s the main device MISO<-. If Daisy's a follower MOSI<-. This is simplex RX.
- `SpiHandle::Config::Direction::ONE_LINE`. Data goes both ways over one line. i.e. MOSI <-> MISO. This is half duplex.

`spi_conf.data_size`
How many bits in each transmission. Defaults to 8.
Must be in the range [4, 32] inclusive.

`spi_conf.clock_polarity`
Is the clock active low or high? 
(Determines if the "first" and "second" edges are rising or falling.)

- `SpiHandle::Config::ClockPolarity::HIGH`. The clock is active high.
- `SpiHandle::Config::ClockPolarity::LOW`. The clock is active low.

Defaults to `SpiHandle::Config::ClockPolarity::LOW`.

`spi_conf.clock_phase`
When is the data ready to be read?
The "first edge" is when we transition __to__ the clock polarity.
The "second edge" is when we transition __away from__ the clock polarity.

So if the polarity is low, the first edge is when the clock goes low.
Again, if the polarity is low, the second edge is when the clock goes high.

- `SpiHandle::Config::ClockPhase::ONE_EDGE` Data is read on the first edge.
- `SpiHandle::Config::ClockPhase::TWO_EDGE`. Data is read on the second edge.

Defaults to `SpiHandle::Config::ClockPhase::ONE_EDGE`.

`spi_conf.nss`
Serial select mode. Must be set by the user.

- `SpiHandle::Config::NSS::SOFT`. Serial select is handled in software. You can ignore the NSS pin.
- `SpiHandle::Config::NSS::HARD_INPUT`. The NSS pin is in use, and the Daisy is a follower.
- `SpiHandle::Config::NSS::HARD_OUTPUT`. The NSS pin is in use, and the Daisy is a leader.

`spi_conf.baud_prescaler`
Division of the default clock rate. 
The clock rate is always 25MHz. 
So with a prescaler of 4 for example, the final clock rate is 25MHz / 4 = 6.25MHz.

Defaults to `SpiHandle::Config::BaudPrescaler::PS_8`

## Blocking Transmit and Receive

Send / receive data in a blocking fashion. The code waits while the transmissions are taking place.

```cpp
// send 4 bytes
uint8_t buffer[4] = {0, 1, 2, 3};
spi_handle.BlockingTransmit(buffer, 4);
```

```cpp
// receive 4 bytes
uint8_t buffer[4];
spi_handle.BlockingReceive(buffer, 4);
```

```cpp
// send and receive 4 bytes
uint8_t tx_buffer[4] = {0, 1, 2, 3};
uint8_t rx_buffer[4];
spi_handle.BlockingTransmitAndReceive(tx_buffer, rx_buffer, 4);
```

## DMA Transmit and Receive

Send / receive data using DMA (Direct Memory Access). 
This allows the hardware to handle the transmission in the background while the code is doing other things.
i.e. it is non blocking.

You can also pass along a callback to be called when the transfer starts, another for when the transfer is over, and a pointer to some data to send those callbacks.

**Note:** Your buffer has to be in the DMA section of memory, as well as in a global scope.

```cpp
// buffer for sending data
uint8_t DMA_BUFFER_MEM_SECTION buffer[4];

// fill the buffer 0, 1, 2, 3
for(uint8_t i = 0; i < 4; i++)
{
    buffer[i] = i;
}

// transmit the data
spi_handle.DmaTransmit(buffer, 4, NULL, NULL, NULL);
```

```cpp
// receive 4 bytes. No callbacks or callback data.
uint8_t DMA_BUFFER_MEM_SECTION buffer[4];
spi_handle.DmaReceive(buffer, 4, NULL, NULL, NULL);
```

```cpp
// send and receive 4 bytes. No callbacks or callback data.
uint8_t DMA_BUFFER_MEM_SECTION tx_buffer[4];
uint8_t DMA_BUFFER_MEM_SECTION rx_buffer[4];

// fill the TX buffer 0, 1, 2, 3
for(uint8_t i = 0; i < 4; i++)
{
   tx_buffer[i] = i;
}

// transmit and receive
spi_handle.DmaTransmitAndReceive(tx_buffer, rx_buffer, 4, NULL, NULL, NULL);
```