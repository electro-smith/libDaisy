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
        spi_cfg.clock_polarity  = SpiHandle::Config::ClockPolarity::HIGH;
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
      if(spi_.BlockingTransmit(data, size) != SpiHandle::Result::OK) {
        printf("uh oh");
      }
    };

  private:
    SpiHandle spi_;
};


template <typename Transport, size_t kNumPixels>
class DotStar
{
  public:
    struct Config
    {
        typename Transport::Config transport_config;
        // TODO: color ordering

        void Defaults()
        {
            transport_config.Defaults();
        };
    };

    DotStar(){};
    ~DotStar(){};

    void Init(Config &config)
    {
        transport_.Init(config.transport_config);
        SetAllGlobalBrightness(15);
        Clear();
    };

    void SetAllGlobalBrightness(uint16_t b)
    {
        for(uint16_t i = 0; i < kNumPixels; i++)
        {
            SetPixelGlobalBrightness(i, b);
        }
    };

    void SetPixelGlobalBrightness(uint16_t idx, uint16_t b)
    {
        if (idx >= kNumPixels) {
          // TODO: return error
          return;
        }
        b = std::min(b, (uint16_t)31);
        uint8_t *pixel = (uint8_t*)(&transmut_buf.pixels[idx]);
        pixel[0] = 0xE0 | b;
    };

    void SetPixelColor(uint16_t idx, uint8_t r, uint8_t g, uint8_t b)
    {
        if(idx >= kNumPixels)
        {
            // TODO: Return error
            return;
        }
        // TODO: Handle color ordering
        uint8_t *pixel = (uint8_t*)(&transmut_buf.pixels[idx]);
        pixel[1] = r;
        pixel[2] = b;
        pixel[3] = g;
    };

    /** \brief Clears all current color data. Does not reset global brightness per pixel.
     */
    void Clear()
    {
        for(uint16_t i = 0; i < kNumPixels; i++)
        {
            uint8_t *pixel = (uint8_t*)(&transmut_buf.pixels[i]);
            pixel[1] = pixel[2] = pixel[3] = 0;
        }
    };

    /** \brief Write current color data to LEDs */
    void Show()
    {
      transport_.Write((uint8_t*)&transmut_buf.sf, 4);
      for (uint16_t i=0; i<kNumPixels; i++) {
        transport_.Write((uint8_t*)&transmut_buf.pixels[i], 4);
      }
      // transport_.Write((uint8_t*)&transmut_buf, sizeof(transmut_buf));
      transport_.Write((uint8_t*)&transmut_buf.ef, 4);
    };

  private:
    Transport transport_;

    // I don't think we need __attribute__((packed)) here
    // because the frames are 32-bit. It could be added to
    // be safer, but that leads to warnings about unaligned
    // pointers/indexing.
    struct {
      const uint32_t sf = 0x00000000;
      uint32_t pixels[kNumPixels];
      const uint32_t ef = 0xFFFFFFFF;
    } transmut_buf;
};

template<size_t kNumPixels>
using DotStarSpi = DotStar<DotStarSpiTransport, kNumPixels>;

} // namespace daisy

#endif