#pragma once

#include "per/spi.h"
#include "per/gpio.h"
#include "sys/system.h"

namespace daisy
{
/**
 * 4 Wire SPI Transport for SSD1327 OLED display devices
 */
class SSD13274WireSpiTransport
{
  public:
    struct Config
    {
        Config()
        {
            // Initialize using defaults
            Defaults();
        }
        SpiHandle::Config spi_config;
        struct
        {
            Pin dc;    /**< Pin used for Data/Command signaling */
            Pin reset; /**< Pin used for Reset */
        } pin_config;
        void Defaults()
        {
            // SPI peripheral config
            spi_config.periph = SpiHandle::Config::Peripheral::SPI_1;
            spi_config.mode   = SpiHandle::Config::Mode::MASTER;
            spi_config.direction
                = SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;
            spi_config.datasize       = 8;
            spi_config.clock_polarity = SpiHandle::Config::ClockPolarity::LOW;
            spi_config.clock_phase    = SpiHandle::Config::ClockPhase::ONE_EDGE;
            spi_config.nss            = SpiHandle::Config::NSS::HARD_OUTPUT;
            spi_config.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_8;
            // SPI pin config
            spi_config.pin_config.sclk = Pin(PORTG, 11);
            spi_config.pin_config.miso = Pin(PORTX, 0);
            spi_config.pin_config.mosi = Pin(PORTB, 5);
            spi_config.pin_config.nss  = Pin(PORTG, 10);
            // SSD1327 control pin config
            pin_config.dc    = Pin(PORTB, 4);
            pin_config.reset = Pin(PORTB, 15);
        }
    };
    void Init(const Config& config)
    {
        // Initialize both GPIO
        pin_dc_.Init(config.pin_config.dc, GPIO::Mode::OUTPUT);
        pin_reset_.Init(config.pin_config.reset, GPIO::Mode::OUTPUT);

        // Initialize SPI
        spi_.Init(config.spi_config);

        // Reset and Configure OLED.
        pin_reset_.Write(false);
        System::Delay(10);
        pin_reset_.Write(true);
        System::Delay(10);
    };
    void SendCommand(uint8_t cmd)
    {
        pin_dc_.Write(false);
        spi_.BlockingTransmit(&cmd, 1);
    };

    void SendData(uint8_t* buff, size_t size)
    {
        pin_dc_.Write(true);
        spi_.BlockingTransmit(buff, size);
    };

  private:
    SpiHandle spi_;
    GPIO      pin_reset_;
    GPIO      pin_dc_;
};


/**
 * A driver implementation for the SSD1327
 */
template <size_t width, size_t height, typename Transport>
class SSD1327Driver
{
  public:
    struct Config
    {
        typename Transport::Config transport_config;
    };

    void Init(Config config)
    {
        color_ = 0x0f;
        transport_.Init(config.transport_config);

        transport_.SendCommand(0x15); // set column address
        transport_.SendCommand(0x00); // start column   0
        transport_.SendCommand(0x3f); // end column 63*2 (two pixels / byte)

        transport_.SendCommand(0x75); // set row address
        transport_.SendCommand(0x00); // start row   0
        transport_.SendCommand(0x7f); // end row   127

        transport_.SendCommand(0x81); // set contrast control
        transport_.SendCommand(0x80);

        transport_.SendCommand(0xa0); // Set Re-map (0x51)
        transport_.SendCommand(
            0x51); // Column Address Remapping, COM Remapping, Splitting of Odd / Even COM Signals

        transport_.SendCommand(0xa1); // start line
        transport_.SendCommand(0x00);

        transport_.SendCommand(0xa2); // display offset
        transport_.SendCommand(0x00);

        transport_.SendCommand(0xa4); // normal display
        transport_.SendCommand(0xa8); // set multiplex ratio
        transport_.SendCommand(0x7f);

        transport_.SendCommand(0xb1); // set phase length
        transport_.SendCommand(0xf1);

        transport_.SendCommand(0xb3); // set dclk
        transport_.SendCommand(
            0x00); // 80Hz:0xc1 / 90Hz:0xe1 / 100Hz:0x00 / 110Hz:0x30 / 120Hz:0x50 / 130Hz:0x70

        transport_.SendCommand(0xab); // Function Selection A
        transport_.SendCommand(0x01);

        transport_.SendCommand(0xb6); // set phase length
        transport_.SendCommand(0x0f);

        transport_.SendCommand(0xbe); // Set VCOMH
        transport_.SendCommand(0x0f);

        transport_.SendCommand(0xbc); // Set Pre-charge voltage
        transport_.SendCommand(0x08);

        transport_.SendCommand(0xd5); // Function Selection B
        transport_.SendCommand(0x62); // Enable second pre-charge

        transport_.SendCommand(0xfd); // unlock command
        transport_.SendCommand(0x12);

        System::Delay(200);           //	wait 200ms
        transport_.SendCommand(0xaf); // turn on display
        Fill(false);
    };

    size_t Width() const { return width; };
    size_t Height() const { return height; };

    void DrawPixel(uint_fast8_t x, uint_fast8_t y, bool on)
    {
        uint8_t  pixel;
        uint32_t line = width / 2;

        if((x >= width) || (y >= height))
            return;

        if(on)
        {
            pixel = buffer_[y * line + (x / 2)];
            if(x % 2)
            {
                pixel &= 0xf0;
                pixel |= color_;
            }
            else
            {
                pixel &= 0x0f;
                pixel |= color_ << 4;
            }
            buffer_[y * line + (x / 2)] = pixel;
        }
        else
        {
            pixel = buffer_[y * line + (x / 2)];
            if(x % 2)
            {
                pixel &= 0xf0;
            }
            else
            {
                pixel &= 0x0f;
            }
            buffer_[y * line + (x / 2)] = pixel;
        }
    };

    void Fill(bool on)
    {
        for(size_t i = 0; i < sizeof(buffer_); i++)
        {
            buffer_[i] = on ? 0xff : 0x00;
        }
    };

    /**
     * Update the display
    */
    void Update()
    {
        transport_.SendCommand(0x15); // column
        transport_.SendCommand(0x00);
        transport_.SendCommand((width / 2) - 1);

        transport_.SendCommand(0x75); // row
        transport_.SendCommand(0x00);
        transport_.SendCommand(height - 1);

        //write data
        transport_.SendData(buffer_, 8192);
    };

    void Set_Color(uint8_t in_col) { color_ = in_col & 0x0f; };

  protected:
    Transport transport_;
    uint8_t   buffer_[width / 2 * height];
    uint8_t   color_;
};

/**
 * A driver for the SSD1327 128x128 OLED displays connected via 4 wire SPI
 */
using SSD13274WireSpi128x128Driver
    = daisy::SSD1327Driver<128, 128, SSD13274WireSpiTransport>;

}; // namespace daisy
