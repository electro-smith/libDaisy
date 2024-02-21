#pragma once
#ifndef SA_OLED_SSD130X_H
#define SA_OLED_SSD130X_H /**< & */

#include "per/i2c.h"
#include "per/spi.h"
#include "per/gpio.h"
#include "sys/system.h"

namespace daisy
{
/**
 * I2C Transport for SSD1306 / SSD1309 OLED display devices
 */
class SSD130xI2CTransport
{
  public:
    struct Config
    {
        Config()
        {
            // Intialize using defaults
            Defaults();
        }
        I2CHandle::Config i2c_config;
        uint8_t           i2c_address;
        void              Defaults()
        {
            i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
            i2c_config.speed          = I2CHandle::Config::Speed::I2C_1MHZ;
            i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
            i2c_config.pin_config.scl = {DSY_GPIOB, 8};
            i2c_config.pin_config.sda = {DSY_GPIOB, 9};
            i2c_address               = 0x3C;
        }
    };
    void Init(const Config& config)
    {
        i2c_address_ = config.i2c_address;
        i2c_.Init(config.i2c_config);
    };
    void SendCommand(uint8_t cmd)
    {
        uint8_t buf[2] = {0X00, cmd};
        i2c_.TransmitBlocking(i2c_address_, buf, 2, 1000);
    };

    void SendData(uint8_t* buff, size_t size)
    {
        for(size_t i = 0; i < size; i++)
        {
            uint8_t buf[2] = {0X40, buff[i]};
            i2c_.TransmitBlocking(i2c_address_, buf, 2, 1000);
        }
    };

  private:
    daisy::I2CHandle i2c_;
    uint8_t          i2c_address_;
};

/**
 * 4 Wire SPI Transport for SSD1306 / SSD1309 OLED display devices
 */
class SSD130x4WireSpiTransport
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
            dsy_gpio_pin dc;    /**< & */
            dsy_gpio_pin reset; /**< & */
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
            spi_config.pin_config.sclk = {DSY_GPIOG, 11};
            spi_config.pin_config.miso = {DSY_GPIOX, 0};
            spi_config.pin_config.mosi = {DSY_GPIOB, 5};
            spi_config.pin_config.nss  = {DSY_GPIOG, 10};
            // SSD130x control pin config
            pin_config.dc    = {DSY_GPIOB, 4};
            pin_config.reset = {DSY_GPIOB, 15};
        }
    };
    void Init(const Config& config)
    {
        // Initialize both GPIO
        pin_dc_.mode = DSY_GPIO_MODE_OUTPUT_PP;
        pin_dc_.pin  = config.pin_config.dc;
        dsy_gpio_init(&pin_dc_);
        pin_reset_.mode = DSY_GPIO_MODE_OUTPUT_PP;
        pin_reset_.pin  = config.pin_config.reset;
        dsy_gpio_init(&pin_reset_);

        // Initialize SPI
        spi_.Init(config.spi_config);

        // Reset and Configure OLED.
        dsy_gpio_write(&pin_reset_, 0);
        System::Delay(10);
        dsy_gpio_write(&pin_reset_, 1);
        System::Delay(10);
    };
    void SendCommand(uint8_t cmd)
    {
        dsy_gpio_write(&pin_dc_, 0);
        spi_.BlockingTransmit(&cmd, 1);
    };

    void SendData(uint8_t* buff, size_t size)
    {
        dsy_gpio_write(&pin_dc_, 1);
        spi_.BlockingTransmit(buff, size);
    };

  private:
    SpiHandle spi_;
    dsy_gpio  pin_reset_;
    dsy_gpio  pin_dc_;
};

/**
 * Soft SPI Transport for SSD1306 / SSD1309 OLED display devices
 */
class SSD130x4WireSoftSpiTransport
{
  public:
    struct Config
    {
        Config()
        {
            // Initialize using defaults
            Defaults();
        }
        struct
        {
            uint32_t     sclk_delay;
            dsy_gpio_pin sclk;
            dsy_gpio_pin mosi;
            dsy_gpio_pin dc;
            dsy_gpio_pin reset;
        } pin_config;
        void Defaults()
        {
            pin_config.sclk_delay = 0; // fast as possible?!
            // SPI peripheral config
            pin_config.sclk = {DSY_GPIOD, 3}; /**< D10 - SPI2 SCK  */
            pin_config.mosi = {DSY_GPIOC, 3}; /**< D9  - SPI2 MOSI */
            // SSD130x control pin config
            pin_config.dc    = {DSY_GPIOC, 11}; //D2
            pin_config.reset = {DSY_GPIOC, 10}; //D3
        }
    };
    void Init(const Config& config)
    {
        // Initialize both GPIO
        pin_sclk_.mode = DSY_GPIO_MODE_OUTPUT_PP;
        pin_sclk_.pin  = config.pin_config.sclk;
        dsy_gpio_init(&pin_sclk_);
        dsy_gpio_write(&pin_sclk_, 1); //ClockPolarity::LOW
        clk_delay      = config.pin_config.sclk_delay;
        pin_mosi_.mode = DSY_GPIO_MODE_OUTPUT_PP;
        pin_mosi_.pin  = config.pin_config.mosi;
        dsy_gpio_init(&pin_mosi_);
        dsy_gpio_write(&pin_mosi_, 0);

        pin_dc_.mode = DSY_GPIO_MODE_OUTPUT_PP;
        pin_dc_.pin  = config.pin_config.dc;
        dsy_gpio_init(&pin_dc_);
        pin_reset_.mode = DSY_GPIO_MODE_OUTPUT_PP;
        pin_reset_.pin  = config.pin_config.reset;
        dsy_gpio_init(&pin_reset_);

        // Reset and Configure OLED.
        dsy_gpio_write(&pin_reset_, 0);
        System::Delay(10);
        dsy_gpio_write(&pin_reset_, 1);
        System::Delay(10);
    };
    void SendCommand(uint8_t cmd)
    {
        dsy_gpio_write(&pin_dc_, 0);
        SoftSpiTransmit(cmd);
    };

    void SendData(uint8_t* buff, size_t size)
    {
        dsy_gpio_write(&pin_dc_, 1);
        for(size_t i = 0; i < size; i++)
            SoftSpiTransmit(buff[i]);
    };

  private:
    void SoftSpiTransmit(uint8_t val)
    {
        // bit flip
        val = ((val & 0x01) << 7) | ((val & 0x02) << 5) | ((val & 0x04) << 3)
              | ((val & 0x08) << 1) | ((val & 0x10) >> 1) | ((val & 0x20) >> 3)
              | ((val & 0x40) >> 5) | ((val & 0x80) >> 7);

        for(uint8_t bit = 0u; bit < 8u; bit++)
        {
            dsy_gpio_write(&pin_mosi_, ((val & (1 << bit)) ? 1 : 0));

            System::DelayTicks(clk_delay);

            dsy_gpio_toggle(&pin_sclk_);

            System::DelayTicks(clk_delay);

            dsy_gpio_toggle(&pin_sclk_);
        }
    }

    uint32_t clk_delay;
    dsy_gpio pin_sclk_;
    dsy_gpio pin_mosi_;
    dsy_gpio pin_reset_;
    dsy_gpio pin_dc_;
};


/**
 * A driver implementation for the SSD1306/SSD1309
 */
template <size_t width, size_t height, typename Transport>
class SSD130xDriver
{
  public:
    struct Config
    {
        typename Transport::Config transport_config;
    };

    void Init(Config config)
    {
        transport_.Init(config.transport_config);

        // Init routine...

        // Display Off
        transport_.SendCommand(0xaE);
        // Dimension dependent commands...
        switch(height)
        {
            case 16:
                // Display Clock Divide Ratio
                transport_.SendCommand(0xD5);
                transport_.SendCommand(0x60);
                // Multiplex Ratio
                transport_.SendCommand(0xA8);
                transport_.SendCommand(0x0F);
                // COM Pins
                transport_.SendCommand(0xDA);
                transport_.SendCommand(0x02);
                break;
            case 32:
                // Display Clock Divide Ratio
                transport_.SendCommand(0xD5);
                transport_.SendCommand(0x80);
                // Multiplex Ratio
                transport_.SendCommand(0xA8);
                transport_.SendCommand(0x1F);
                // COM Pins
                transport_.SendCommand(0xDA);
                if(width == 64)
                {
                    transport_.SendCommand(0x12);
                }
                else
                {
                    transport_.SendCommand(0x02);
                }

                break;
            case 48:
                // Display Clock Divide Ratio
                transport_.SendCommand(0xD5);
                transport_.SendCommand(0x80);
                // Multiplex Ratio
                transport_.SendCommand(0xA8);
                transport_.SendCommand(0x2F);
                // COM Pins
                transport_.SendCommand(0xDA);
                transport_.SendCommand(0x12);
                break;
            default: // 128
                // Display Clock Divide Ratio
                transport_.SendCommand(0xD5);
                transport_.SendCommand(0x80);
                // Multiplex Ratio
                transport_.SendCommand(0xA8);
                transport_.SendCommand(0x3F);
                // COM Pins
                transport_.SendCommand(0xDA);
                transport_.SendCommand(0x12);
                break;
        }

        // Display Offset
        transport_.SendCommand(0xD3);
        transport_.SendCommand(0x00);
        // Start Line Address
        transport_.SendCommand(0x40);
        // Normal Display
        transport_.SendCommand(0xA6);
        // All On Resume
        transport_.SendCommand(0xA4);
        // Charge Pump
        transport_.SendCommand(0x8D);
        transport_.SendCommand(0x14);
        // Set Segment Remap
        transport_.SendCommand(0xA1);
        // COM Output Scan Direction
        transport_.SendCommand(0xC8);
        // Contrast Control
        transport_.SendCommand(0x81);
        transport_.SendCommand(0x8F);
        // Pre Charge
        transport_.SendCommand(0xD9);
        transport_.SendCommand(0x25);
        // VCOM Detect
        transport_.SendCommand(0xDB);
        transport_.SendCommand(0x34);


        // Display On
        transport_.SendCommand(0xAF); //--turn on oled panel
    };

    size_t Width() const { return width; };
    size_t Height() const { return height; };

    void DrawPixel(uint_fast8_t x, uint_fast8_t y, bool on)
    {
        if(x >= width || y >= height)
            return;
        if(on)
            buffer_[x + (y / 8) * width] |= (1 << (y % 8));
        else
            buffer_[x + (y / 8) * width] &= ~(1 << (y % 8));
    }

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
        uint8_t i;
        uint8_t high_column_addr;
        switch(height)
        {
            case 32: high_column_addr = 0x12; break;

            default: high_column_addr = 0x10; break;
        }
        for(i = 0; i < (height / 8); i++)
        {
            transport_.SendCommand(0xB0 + i);
            transport_.SendCommand(0x00);
            transport_.SendCommand(high_column_addr);
            transport_.SendData(&buffer_[width * i], width);
        }
    };

  protected:
    Transport transport_;
    uint8_t   buffer_[width * height / 8];
};

/**
 * A driver for the SSD1306/SSD1309 128x64 OLED displays connected via 4 wire SPI  
 */
using SSD130x4WireSpi128x64Driver
    = daisy::SSD130xDriver<128, 64, SSD130x4WireSpiTransport>;

/**
 * A driver for the SSD1306/SSD1309 128x32 OLED displays connected via 4 wire SPI  
 */
using SSD130x4WireSpi128x32Driver
    = daisy::SSD130xDriver<128, 32, SSD130x4WireSpiTransport>;

/**
 * A driver for the SSD1306/SSD1309 98x16 OLED displays connected via 4 wire SPI  
 */
using SSD130x4WireSpi98x16Driver
    = daisy::SSD130xDriver<98, 16, SSD130x4WireSpiTransport>;

/**
 * A driver for the SSD1306/SSD1309 64x48 OLED displays connected via 4 wire SPI  
 */
using SSD130x4WireSpi64x48Driver
    = daisy::SSD130xDriver<64, 48, SSD130x4WireSpiTransport>;

/**
 * A driver for the SSD1306/SSD1309 64x32 OLED displays connected via 4 wire SPI  
 */
using SSD130x4WireSpi64x32Driver
    = daisy::SSD130xDriver<64, 32, SSD130x4WireSpiTransport>;

/**
 * A driver for the SSD1306/SSD1309 128x64 OLED displays connected via I2C  
 */
using SSD130xI2c128x64Driver
    = daisy::SSD130xDriver<128, 64, SSD130xI2CTransport>;

/**
 * A driver for the SSD1306/SSD1309 128x32 OLED displays connected via I2C  
 */
using SSD130xI2c128x32Driver
    = daisy::SSD130xDriver<128, 32, SSD130xI2CTransport>;

/**
 * A driver for the SSD1306/SSD1309 98x16 OLED displays connected via I2C  
 */
using SSD130xI2c98x16Driver = daisy::SSD130xDriver<98, 16, SSD130xI2CTransport>;

/**
 * A driver for the SSD1306/SSD1309 64x48 OLED displays connected via I2C  
 */
using SSD130xI2c64x48Driver = daisy::SSD130xDriver<64, 48, SSD130xI2CTransport>;

/**
 * A driver for the SSD1306/SSD1309 64x32 OLED displays connected via I2C  
 */
using SSD130xI2c64x32Driver = daisy::SSD130xDriver<64, 32, SSD130xI2CTransport>;

/**
 * A driver for the SSD1306/SSD1309 128x64 OLED displays connected via 4 wire SPI  
 */
using SSD130x4WireSoftSpi128x64Driver
    = daisy::SSD130xDriver<128, 64, SSD130x4WireSoftSpiTransport>;

}; // namespace daisy


#endif
