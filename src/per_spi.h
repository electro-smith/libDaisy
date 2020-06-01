#pragma once
#ifndef DSY_SPI_H
#define DSY_SPI_H

#include "daisy_core.h"

/** TODO:
- Add documentation
- Add configuration
- Add reception
- Add IT
- Add DMA
*/


namespace daisy
{

enum SpiPeriph
{
    SPI_PERIPH_1,
    SPI_PERIPH_3,
    SPI_PERIPH_6,
};

enum SpiPin
{
    SPI_PIN_CS,
    SPI_PIN_SCK,
    SPI_PIN_MOSI,
    SPI_PIN_MISO,
};


class SpiHandle
{
  public:
    SpiHandle() {}
    ~SpiHandle() {}

    void Init();

    void BlockingTransmit(uint8_t *buff, size_t size);

  private:
};


} // namespace daisy

#endif
