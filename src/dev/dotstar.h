#pragma once
#ifndef DSY_APA120_H
#define DSY_APA120_H

#include "per/i2c.h"
#include "per/spi.h"


// DotStar color ordering permutations
// Offset:            R          G          B
#define DOTSTAR_RGB ((0 << 4) | (1 << 2) | (2))
#define DOTSTAR_RBG ((0 << 4) | (2 << 2) | (1))
#define DOTSTAR_GRB ((1 << 4) | (0 << 2) | (2))
#define DOTSTAR_GBR ((2 << 4) | (0 << 2) | (1))
#define DOTSTAR_BRG ((1 << 4) | (2 << 2) | (0))
#define DOTSTAR_BGR ((2 << 4) | (1 << 2) | (0))

namespace daisy
{

class DotStarSpiTransport
{
  public:
    struct Config
    {
        SpiHandle::Config::Peripheral    periph;
        SpiHandle::Config::BaudPrescaler prescale;
        Pin                              clk_pin;
        Pin                              data_pin;

        void Defaults()
        {
            periph   = SpiHandle::Config::Peripheral::SPI_1;
            prescale = SpiHandle::Config::BaudPrescaler::PS_4;
            clk_pin  = Pin(PORTG, 11);
            data_pin = Pin(PORTB, 5);
        };
    };

    inline void Init(Config &config)
    {
        SpiHandle::Config spi_cfg;
        spi_cfg.periph    = config.periph;
        spi_cfg.mode      = SpiHandle::Config::Mode::MASTER;
        spi_cfg.direction = SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;
        spi_cfg.clock_polarity  = SpiHandle::Config::ClockPolarity::LOW;
        spi_cfg.clock_phase     = SpiHandle::Config::ClockPhase::ONE_EDGE;
        spi_cfg.datasize        = 8;
        spi_cfg.nss             = SpiHandle::Config::NSS::SOFT;
        spi_cfg.baud_prescaler  = config.prescale;
        spi_cfg.pin_config.sclk = config.clk_pin;
        spi_cfg.pin_config.mosi = config.data_pin;
        spi_cfg.pin_config.miso = Pin();
        spi_cfg.pin_config.nss  = Pin();

        spi_.Init(spi_cfg);
    };

    void Write(uint8_t *data, size_t size)
    {
        if(spi_.BlockingTransmit(data, size) != SpiHandle::Result::OK)
        {
            // TODO: Error handling
        }
    };

  private:
    SpiHandle spi_;
};

template <typename Transport>
class DotStar
{
  public:
    struct Config
    {
        typename Transport::Config transport_config;
        uint16_t                   num_pixels;
        // TODO: color ordering

        void Defaults()
        {
            transport_config.Defaults();
            num_pixels = 1;
        };
    };

    DotStar(){};
    ~DotStar(){};

    void Init(Config &config)
    {
        if(config.num_pixels > kMaxNumPixels)
        {
            // TODO: Error Handling
            return;
        }
        transport_.Init(config.transport_config);
        num_pixels_ = config.num_pixels;
        SetAllGlobalBrightness(15);
        Clear();
    };

    /**
     * \brief Set global brightness for all pixels
     * \details "Global brighntess" for the APA120 device sets the
     *          equivalent constant current for the LEDs. See datasheet
     *          for details.
     *
     * \param b 5-bit global brightness setting (0 - 31)
     */
    void SetAllGlobalBrightness(uint16_t b)
    {
        for(uint16_t i = 0; i < num_pixels_; i++)
        {
            SetPixelGlobalBrightness(i, b);
        }
    };

    /**
     * \brief Set global brightness for a single pixel
     * \details "Global brighntess" for the APA120 device sets the
     *          equivalent constant current for the LEDs. See datasheet
     *          for details.
     *
     * \param idx Index of the pixel for which to set global brightness
     * \param b 5-bit global brightness setting (0 - 31)
     */
    void SetPixelGlobalBrightness(uint16_t idx, uint16_t b)
    {
        if(idx >= num_pixels_)
        {
            // TODO: return error
            return;
        }
        uint8_t *pixel = (uint8_t *)(&pixels_[idx]);
        pixel[0]       = 0xE0 | std::min(b, (uint16_t)31);
    };

    void SetPixelColor(uint16_t idx, const Color &color)
    {
        SetPixelColor(idx, color.Red8(), color.Green8(), color.Blue8());
    }

    void SetPixelColor(uint16_t idx, uint8_t r, uint8_t g, uint8_t b)
    {
        if(idx >= num_pixels_)
        {
            // TODO: Return error
            return;
        }
        // TODO: Handle color ordering
        uint8_t *pixel = (uint8_t *)(&pixels_[idx]);
        pixel[1]       = r;
        pixel[2]       = b;
        pixel[3]       = g;
    };

    /** \brief Clears all current color data. Does not reset global brightnesses.
     */
    void Clear()
    {
        for(uint16_t i = 0; i < num_pixels_; i++)
        {
            SetPixelColor(i, 0, 0, 0);
        }
    };

    /** \brief Write current color data to LEDs */
    void Show()
    {
        uint8_t sf[4] = {0x00, 0x00, 0x00, 0x00};
        uint8_t ef[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        transport_.Write(sf, 4);
        for(uint16_t i = 0; i < num_pixels_; i++)
        {
            transport_.Write((uint8_t *)&pixels_[i], 4);
        }
        transport_.Write(ef, 4);
    };

  private:
    static const size_t kMaxNumPixels = 64;
    Transport           transport_;
    uint16_t            num_pixels_;
    uint32_t            pixels_[kMaxNumPixels];
};

using DotStarSpi = DotStar<DotStarSpiTransport>;

} // namespace daisy

#endif