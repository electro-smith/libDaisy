#ifndef __OLED_SH1106_H__
#define __OLED_SH1106_H__

#include "dev/oled_ssd130x.h"

namespace daisy
{
/**
 * A driver implementation for SH1106 OLED displays
 */
template <size_t width, size_t height, typename Transport>
class SH1106Driver : public SSD130xDriver<width, height, Transport>
{
  public:
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
            this->transport_.SendCommand(0xB0 + i);
            this->transport_.SendCommand(0x02);
            this->transport_.SendCommand(high_column_addr);
            this->transport_.SendData(&this->buffer_[width * i], width);
        }
    };
};

/**
 * A driver for SH1106 128x64 OLED displays connected via 4 wire SPI
 */
using SH11064WireSpi128x64Driver
    = SH1106Driver<128, 64, SSD130x4WireSpiTransport>;

/**
 * A driver for SH1106 128x64 OLED displays connected via I2C
 */
using SH1106I2c128x64Driver = SH1106Driver<128, 64, SSD130xI2CTransport>;

}; // namespace daisy

#endif
