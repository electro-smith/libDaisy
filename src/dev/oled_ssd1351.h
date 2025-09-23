#pragma once

#include "per/spi.h"
#include "per/gpio.h"
#include "sys/system.h"

#define oled_white 0xffff
#define oled_black 0x0000
#define oled_red 0x00f1
#define oled_green 0xe007
#define oled_blue 0x1f00
#define oled_cyan (oled_green | oled_blue)
#define oled_yellow (oled_green | oled_red)
#define oled_magenta (oled_red | oled_blue)

namespace daisy
{
/**
 * 4 Wire SPI Transport for SSD1351 OLED display devices
 */
class SSD13514WireSpiTransport
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
            // SSD1351 control pin config
            pin_config.dc    = Pin(PORTB, 4);
            pin_config.reset = Pin(PORTB, 15);
        }
    };
    void Init(const Config& config)
    {
        // Initialize both GPIO
        pin_reset_.Init(config.pin_config.reset, GPIO::Mode::OUTPUT);
        pin_dc_.Init(config.pin_config.dc, GPIO::Mode::OUTPUT);

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

    void SendData(uint8_t data)
    {
        pin_dc_.Write(true);
        spi_.BlockingTransmit(&data, 1);
    };

  private:
    SpiHandle spi_;
    GPIO      pin_reset_;
    GPIO      pin_dc_;
};


/**
 * A driver implementation for the SSD1351
 */
template <size_t width, size_t height, typename Transport>
class SSD1351Driver
{
  public:
    struct Config
    {
        typename Transport::Config transport_config;
    };

    void Init(Config config)
    {
        fg_color_ = oled_white;
        bg_color_ = oled_black;
        transport_.Init(config.transport_config);

        transport_.SendCommand(0xfd); // lock IC
        transport_.SendData(0x12);
        transport_.SendCommand(0xfd); // unlock IC
        transport_.SendData(0xb1);    //

        transport_.SendCommand(0xae); // display off

        transport_.SendCommand(0x15); // set column address
        transport_.SendData(0x00);    // column address start 00
        transport_.SendData(0x7f);    // column address end 127

        transport_.SendCommand(0x75); // set row address
        transport_.SendData(0x00);    // row address start 00
        transport_.SendData(0x7f);    // row address end 127

        transport_.SendCommand(
            0xB3); // Set Front Clock Divider / Oscillator Frequency
        transport_.SendData(0xF1);

        transport_.SendCommand(0xCA); // Set Multiplex Ratio
        transport_.SendData(0x7F);

        transport_.SendCommand(0xa0); // Set Re-map & Dual COM Line Mode
        transport_.SendData(
            0x74); // color mode 64k, enable com split, reverse com scan, color swapped, hz scan

        transport_.SendCommand(0xa1); // set display start line
        transport_.SendData(0x00);    // line 0

        transport_.SendCommand(0xa2); // set display offset
        transport_.SendData(0x00);    // column 0

        transport_.SendCommand(0xAB); // Function Selection
        transport_.SendData(0x01);

        transport_.SendCommand(0xB4); // Set Segment Low Voltage
        transport_.SendData(0xA0);
        transport_.SendData(0xB5);
        transport_.SendData(0x55);

        transport_.SendCommand(0xC1); // Set Contrast Current for Color A,B,C
        transport_.SendData(0xC8);
        transport_.SendData(0x80);
        transport_.SendData(0xC0);

        transport_.SendCommand(0xC7); // Master Contrast Current Control
        transport_.SendData(0x0F);

        transport_.SendCommand(
            0xB1); // Set Reset (Phase 1) / Pre-charge (Phase 2) period
        transport_.SendData(0x32);

        transport_.SendCommand(0xB2); // Display Enhancement
        transport_.SendData(0xA4);
        transport_.SendData(0x00);
        transport_.SendData(0x00);

        transport_.SendCommand(0xBB); // Set Pre-charge voltage
        transport_.SendData(0x17);

        transport_.SendCommand(0xB6); // Set Second Precharge Period
        transport_.SendData(0x01);

        transport_.SendCommand(0xBE); //	Set VCOMH Voltage
        transport_.SendData(0x05);

        transport_.SendCommand(0xA6); // Normal display

        System::Delay(300);           //	wait 300ms
        transport_.SendCommand(0xaf); // turn on display
        Fill(false);
    };

    size_t Width() const { return width; };
    size_t Height() const { return height; };

    void DrawPixel(uint_fast8_t x, uint_fast8_t y, bool on)
    {
        if((x >= width) || (y >= height))
            return;

        if(on)
        {
            buffer_[(y * width) + x] = fg_color_;
        }
        else
        {
            buffer_[(y * width) + x] = bg_color_;
        }
    };

    void Fill(bool on)
    {
        for(size_t i = 0; i < sizeof(buffer_) / 2; i++)
        {
            buffer_[i] = on ? fg_color_ : bg_color_;
        }
    };

    /**
     * Update the display
    */
    void Update()
    {
        transport_.SendCommand(0x15); // column
        transport_.SendData(0x00);
        transport_.SendData(width - 1);

        transport_.SendCommand(0x75); // row
        transport_.SendData(0x00);
        transport_.SendData(height - 1);

        transport_.SendCommand(0x5c); // write display buffer
        transport_.SendData((uint8_t*)buffer_, sizeof(buffer_));
    };

    void SetColorFG(uint8_t red, uint8_t green, uint8_t blue)
    {
        uint16_t t1, t2;

        fg_color_ = (red & 0x1f) << 11 | (green & 0x3f) << 5 | (blue & 0x1f);
        t1        = (fg_color_ >> 8) & 0xff;
        t2        = (fg_color_ & 0xff);
        fg_color_ = t2 << 8 | t1;
    };

    void SetColorBG(uint8_t red, uint8_t green, uint8_t blue)
    {
        uint16_t t1, t2;

        bg_color_ = (red & 0x1f) << 11 | (green & 0x3f) << 5 | (blue & 0x1f);
        t1        = (bg_color_ >> 8) & 0xff;
        t2        = (bg_color_ & 0xff);
        bg_color_ = t2 << 8 | t1;
    };

  protected:
    Transport transport_;
    uint16_t  buffer_[width * height];
    uint16_t  fg_color_;
    uint16_t  bg_color_;
};

/**
 * A driver for the SSD1351 128x128 OLED displays connected via 4 wire SPI
 */
using SSD13514WireSpi128x128Driver
    = daisy::SSD1351Driver<128, 128, SSD13514WireSpiTransport>;

}; // namespace daisy
