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

/** SPI peripheral enum */
enum SpiPeriph
{
    SPI_PERIPH_1, /**< SPI peripheral 1 */
    SPI_PERIPH_3, /**< SPI peripheral 3 */
    SPI_PERIPH_6, /**< SPI peripheral 3 */
};

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
