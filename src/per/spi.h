#pragma once
#ifndef DSY_SPI_H
#define DSY_SPI_H

#include "daisy_core.h"

/* TODO:
- Add documentation
- Add configuration
- Add reception
- Add IT
- Add DMA
*/

namespace daisy
{
/** @addtogroup serial
@{
*/

/** SPI pins */
enum SpiPin
{
    SPI_PIN_CS,   /**< CS pin */
    SPI_PIN_SCK,  /**< SCK pin */
    SPI_PIN_MOSI, /**< MOSI pin */
    SPI_PIN_MISO, /**< MISO pin */
};

/**  Handler for serial peripheral interface */
class SpiHandle
{
  public:
    struct Config{
      enum class Peripheral
      {
          SPI_PERIPH_1,
          SPI_PERIPH_2,
          SPI_PERIPH_3,
          SPI_PERIPH_4,
          SPI_PERIPH_5,
          SPI_PERIPH_6,
      };

      Peripheral periph;
    };

    SpiHandle() : pimpl_(nullptr) {}
    SpiHandle(const SpiHandle& other) = default;
    SpiHandle& operator=(const SpiHandle& other) = default;

    /** Return values for Uart functions. */
    enum class Result
    {
        OK, /**< & */
        ERR /**< & */
    };

    /** Initializes handler */
    Result Init(const Config& config);

    /** Blocking transmit 
    \param *buff input buffer
    \param size  buffer size
    */
    Result BlockingTransmit(uint8_t *buff, size_t size);

    class Impl; /**< SPI implementation */

  private:
    Impl* pimpl_;
};


/** @} */
} // namespace daisy

#endif
