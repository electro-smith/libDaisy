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

    SpiHandle() {}
    ~SpiHandle() {}

    /** Initializes handler */
    void Init();

    /** Blocking transmit 
    \param *buff input buffer
    \param size  buffer size
    */
    void BlockingTransmit(uint8_t *buff, size_t size);

  private:
};


/** @} */
} // namespace daisy

#endif
