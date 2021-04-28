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
    void Init(const I2CHandle::Config& i2c_conf, uint8_t i2c_address = 0x3C)
    {
        i2c_address_ = i2c_address;
        i2c_.Init(i2c_conf);
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
class SSD130xSPITransport
{
  public:
    enum Pins
    {
        DATA_COMMAND, /**< Data command pin. */
        RESET,        /**< Reset pin */
        NUM_PINS,     /**< Num pins */
    };
    void Init(dsy_gpio_pin* pin_cfg)
    {
        // Initialize both GPIO
        pin_dc_.mode = DSY_GPIO_MODE_OUTPUT_PP;
        pin_dc_.pin  = pin_cfg[DATA_COMMAND];
        dsy_gpio_init(&pin_dc_);
        pin_reset_.mode = DSY_GPIO_MODE_OUTPUT_PP;
        pin_reset_.pin  = pin_cfg[RESET];
        dsy_gpio_init(&pin_reset_);
        // Initialize SPI
        spi_.Init();

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
 * A driver implementation for the SSD1306/SSD1309
 */
template <typename Transport>
class SSD130xDriver
{
  public:
    enum PixelDimensions
    {
        W128_H64,
        W128_H32,
        W96_H16,
        W64_H48,
        W64_H32
    };

    void Init(const Transport& transport, PixelDimensions dimensions = W128_H64)
    {
        transport_ = transport;

        switch(dimensions)
        {
            case W128_H64:
                width_  = 128;
                height_ = 64;
                break;
            case W128_H32:
                width_  = 128;
                height_ = 32;
                break;
            case W96_H16:
                width_  = 96;
                height_ = 16;
                break;
            case W64_H48:
                width_  = 64;
                height_ = 48;
                break;
            case W64_H32:
                width_  = 64;
                height_ = 32;
                break;
            default:
                width_  = 128;
                height_ = 64;
                break;
        }


        // Display Off
        transport_.SendCommand(0xaE);
        // Dimension dependent commands...
        switch(dimensions)
        {
            case W128_H64:
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
            case W128_H32:
                // Display Clock Divide Ratio
                transport_.SendCommand(0xD5);
                transport_.SendCommand(0x80);
                // Multiplex Ratio
                transport_.SendCommand(0xA8);
                transport_.SendCommand(0x1F);
                // COM Pins
                transport_.SendCommand(0xDA);
                transport_.SendCommand(0x02);
                break;
            case W96_H16:
                // Display Clock Divide Ratio
                transport_.SendCommand(0xd5);
                transport_.SendCommand(0x60);
                // Multiplex Ratio
                transport_.SendCommand(0xA8);
                transport_.SendCommand(0x0F);
                // COM Pins
                transport_.SendCommand(0xDA);
                transport_.SendCommand(0x02);
                break;
            case W64_H48:
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
            case W64_H32:
                // Display Clock Divide Ratio
                transport_.SendCommand(0xD5);
                transport_.SendCommand(0x80);
                // Multiplex Ratio
                transport_.SendCommand(0xA8);
                transport_.SendCommand(0x1F);
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

    uint16_t Width() { return width_; };
    uint16_t Height() { return height_; };
    uint16_t CurrentX() { return current_x_; };
    uint16_t CurrentY() { return current_y_; };

    void SetCursor(uint8_t x, uint8_t y)
    {
        current_x_ = x;
        current_y_ = y;
    }

    void DrawPixel(uint_fast8_t x, uint_fast8_t y, bool on)
    {
        if(x >= width_ || y >= height_)
            return;
        if(on)
            buffer_[x + (y / 8) * width_] |= (1 << (y % 8));
        else
            buffer_[x + (y / 8) * width_] &= ~(1 << (y % 8));
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
        switch(height_)
        {
            case 32: high_column_addr = 0x12; break;

            default: high_column_addr = 0x10; break;
        }
        for(i = 0; i < (height_ / 8); i++)
        {
            transport_.SendCommand(0xB0 + i);
            transport_.SendCommand(0x00);
            transport_.SendCommand(high_column_addr);
            transport_.SendData(&buffer_[width_ * i], width_);
        }
    };

  private:
    PixelDimensions dimensions_;
    uint16_t        width_;
    uint16_t        height_;
    uint16_t        current_x_;
    uint16_t        current_y_;
    Transport       transport_;
    // Create a display buffer according to the largest possible display dimensions.
    // TODO - is there a way to statically initialize this dependent on display dimensions?
    uint8_t buffer_[128 * 64 / 8];
};

}; // namespace daisy


#endif