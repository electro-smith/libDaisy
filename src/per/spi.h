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
          SPI_1,
          SPI_2,
          SPI_3,
          SPI_4,
          SPI_5,
          SPI_6,
      };

      enum class Mode
      {
        MASTER,
        SLAVE,
      };

      enum class Direction{
        TWO_LINES,
        TWO_LINES_TX_ONLY,
        TWO_LINES_RX_ONLY,
        ONE_LINE,
      };

      enum class ClockPolarity{
        LOW,
        HIGH,
      };

      enum class ClockPhase{
        ONE_EDGE,
        TWO_EDGE,
      };

      enum class NSS{
        SOFT,
        HARD_INPUT,
        HARD_OUTPUT,
      };

      enum class BaudPrescaler{
        BAUDRATEPRESCALER_2,
        BAUDRATEPRESCALER_4,
        BAUDRATEPRESCALER_8,
        BAUDRATEPRESCALER_16,
        BAUDRATEPRESCALER_32,
        BAUDRATEPRESCALER_64,
        BAUDRATEPRESCALER_128,
        BAUDRATEPRESCALER_256,
      };

      Peripheral periph;
      Mode mode;
      Direction direction;
      unsigned long datasize;
      ClockPolarity clock_polarity;
      ClockPhase clock_phase;
      NSS nss;
      BaudPrescaler baud_prescaler;
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
