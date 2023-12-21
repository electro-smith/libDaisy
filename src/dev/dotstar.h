#pragma once
#ifndef DSY_DOTSTAR_H
#define DSY_DOTSTAR_H

#include "per/i2c.h"
#include "per/spi.h"

namespace daisy
{
/**
 * \brief SPI Transport for DotStars
 */
class DotStarSpiTransport
{
  public:
    struct Config
    {
        SpiHandle::Config::Peripheral    periph;
        SpiHandle::Config::BaudPrescaler baud_prescaler;
        Pin                              clk_pin;
        Pin                              data_pin;

        void Defaults()
        {
            periph         = SpiHandle::Config::Peripheral::SPI_1;
            baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_4;
            clk_pin        = Pin(PORTG, 11);
            data_pin       = Pin(PORTB, 5);
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
        spi_cfg.baud_prescaler  = config.baud_prescaler;
        spi_cfg.pin_config.sclk = config.clk_pin;
        spi_cfg.pin_config.mosi = config.data_pin;
        spi_cfg.pin_config.miso = Pin();
        spi_cfg.pin_config.nss  = Pin();

        spi_.Init(spi_cfg);
    };

    bool Write(uint8_t *data, size_t size)
    {
        return spi_.BlockingTransmit(data, size) == SpiHandle::Result::OK;
    };

  private:
    SpiHandle spi_;
};


/** \brief Device support for Adafruit DotStar LEDs (Opsco SK9822)
    \author Nick Donaldson
    \date March 2023
*/
template <typename Transport>
class DotStar
{
  public:
    enum class Result
    {
        OK,
        ERR_INVALID_ARGUMENT,
        ERR_TRANSPORT
    };

    struct Config
    {
        enum ColorOrder : uint8_t
        {
            //      R          G          B
            RGB = ((0 << 4) | (1 << 2) | (2)),
            RBG = ((0 << 4) | (2 << 2) | (1)),
            GRB = ((1 << 4) | (0 << 2) | (2)),
            GBR = ((2 << 4) | (0 << 2) | (1)),
            BRG = ((1 << 4) | (2 << 2) | (0)),
            BGR = ((2 << 4) | (1 << 2) | (0)),
        };

        typename Transport::Config
                   transport_config; /**< Transport-specific configuration */
        ColorOrder color_order;      /**< Pixel color channel ordering */
        uint16_t   num_pixels;       /**< Number of pixels/LEDs (max 64) */

        void Defaults()
        {
            transport_config.Defaults();
            color_order = ColorOrder::RGB;
            num_pixels  = 1;
        };
    };

    DotStar(){};
    ~DotStar(){};

    Result Init(Config &config)
    {
        if(config.num_pixels > kMaxNumPixels)
        {
            return Result::ERR_INVALID_ARGUMENT;
        }
        transport_.Init(config.transport_config);
        num_pixels_ = config.num_pixels;
        // first color byte is always global brightness (hence +1 offset)
        r_offset_ = ((config.color_order >> 4) & 0b11) + 1;
        g_offset_ = ((config.color_order >> 2) & 0b11) + 1;
        b_offset_ = (config.color_order & 0b11) + 1;
        // Minimum brightness by default. These LEDs can
        // be very current hungry. See datasheet for details.
        SetAllGlobalBrightness(1);
        Clear();
        return Result::OK;
    };

    /**
     * \brief Set global brightness for all pixels
     * \details "Global brightness" for the SK9822 device sets the
     *          equivalent constant current for the LEDs, not a pre-multiplied PWM
     *          brightness scaling for the pixel's RGB value. See SK9822 datasheet
     *          for details.
     * \warning Recommend not going above 10, especially for SK9822-EC20 which may
     *          overheat if you do.
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
     * \details "Global brightness" for the SK9822 device sets the
     *          equivalent constant current for the LEDs. See datasheet
     *          for details.
     * \warning Recommend not going above 10, especially for SK9822-EC20 which may
     *          overheat if you do.
     *
     * \param idx Index of the pixel for which to set global brightness
     * \param b 5-bit global brightness setting (0 - 31)
     */
    Result SetPixelGlobalBrightness(uint16_t idx, uint16_t b)
    {
        if(idx >= num_pixels_)
        {
            return Result::ERR_INVALID_ARGUMENT;
        }
        uint8_t *pixel = (uint8_t *)(&pixels_[idx]);
        pixel[0]       = 0xE0 | std::min(b, (uint16_t)31);
        return Result::OK;
    };

    uint16_t GetPixelColor(uint16_t idx)
    {
        if(idx >= num_pixels_)
            return 0;
        uint32_t       c     = 0;
        const uint8_t *pixel = (uint8_t *)&pixels_[idx];
        c                    = c | (pixel[r_offset_] << 16);
        c                    = c | (pixel[g_offset_] << 8);
        c                    = c | pixel[b_offset_];
        return c;
    }

    /**
     * \brief Sets color of a single pixel
     *
     * \param idx Index of the pixel
     * \param color Color object to apply to the pixel
     */
    void SetPixelColor(uint16_t idx, const Color &color)
    {
        SetPixelColor(idx, color.Red8(), color.Green8(), color.Blue8());
    }

    /**
     * \brief Sets color of a single pixel
     * \param color 32-bit integer representing 24-bit RGB color. MSB ignored.
     */
    void SetPixelColor(uint16_t idx, uint32_t color)
    {
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        SetPixelColor(idx, r, g, b);
    }

    /**
     * \brief Sets color of a single pixel
     *
     * \param idx Index of the pixel
     * \param r 8-bit red value to apply to pixel
     * \param g 8-bit green value to apply to pixel
     * \param b 8-bit blue value to apply to pixel
     */
    Result SetPixelColor(uint16_t idx, uint8_t r, uint8_t g, uint8_t b)
    {
        if(idx >= num_pixels_)
        {
            return Result::ERR_INVALID_ARGUMENT;
        }
        uint8_t *pixel   = (uint8_t *)(&pixels_[idx]);
        pixel[r_offset_] = r;
        pixel[b_offset_] = b;
        pixel[g_offset_] = g;
        return Result::OK;
    };

    /**
     * \brief Fills all pixels with color
     * \param color Color with which to fill all pixels
     */
    void Fill(const Color &color)
    {
        for(uint16_t i = 0; i < num_pixels_; i++)
        {
            SetPixelColor(i, color);
        }
    }

    /**
     * \brief Fills all pixels with color
     * \param color 32-bit integer representing 24-bit RGB color. MSB ignored.
     */
    void Fill(uint32_t color)
    {
        for(uint16_t i = 0; i < num_pixels_; i++)
        {
            SetPixelColor(i, color);
        }
    }

    /**
     * \brief Fill all pixels with color
     * \param r 8-bit red value to apply to pixels
     * \param g 8-bit green value to apply to pixels
     * \param b 8-bit blue value to apply to pixels
     */
    void Fill(uint8_t r, uint8_t g, uint8_t b)
    {
        for(uint16_t i = 0; i < num_pixels_; i++)
        {
            SetPixelColor(i, r, g, b);
        }
    }

    /** \brief Clears all current color data.
     *         Does not reset global brightnesses.
     *         Does not write pixel buffer data to LEDs.
     */
    void Clear()
    {
        for(uint16_t i = 0; i < num_pixels_; i++)
        {
            SetPixelColor(i, 0);
        }
    };

    /** \brief Writes current pixel buffer data to LEDs */
    Result Show()
    {
        uint8_t sf[4] = {0x00, 0x00, 0x00, 0x00};
        uint8_t ef[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        if(!transport_.Write(sf, 4))
        {
            return Result::ERR_TRANSPORT;
        }
        for(uint16_t i = 0; i < num_pixels_; i++)
        {
            if(!transport_.Write((uint8_t *)&pixels_[i], 4))
            {
                return Result::ERR_TRANSPORT;
            }
        }
        if(!transport_.Write(ef, 4))
        {
            return Result::ERR_TRANSPORT;
        }
        return Result::OK;
    };

  private:
    static const size_t kMaxNumPixels = 64;
    Transport           transport_;
    uint16_t            num_pixels_;
    uint32_t            pixels_[kMaxNumPixels];
    uint8_t             r_offset_, g_offset_, b_offset_;
};

using DotStarSpi = DotStar<DotStarSpiTransport>;

} // namespace daisy

#endif
