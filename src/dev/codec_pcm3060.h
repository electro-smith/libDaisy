#pragma once
#ifndef DSY_CODEC_PCM3060_H
#define DSY_CODEC_PCM3060_H
#include "per/i2c.h"
namespace daisy
{
/**
 * @brief Driver for the TI PCM3060 Audio Codec.
 * @addtogroup codec
 * 
 * For now this is a limited interface that uses I2C to communicate with the PCM3060
 * The device can also be accessed with SPI, which is not yet supported.
 * 
 * For now all registers are set to their defaults, and the Init function will 
 * perform a MRST and SRST before setting the format to 24bit LJ, and disabling 
 * power save for both the ADC and DAC.
 *
 */
class Pcm3060
{
  public:
    enum class Result
    {
        OK,
        ERR,
    };

    Pcm3060() {}
    ~Pcm3060() {}

    /** Initializes the PCM3060 in 24-bit MSB aligned I2S mode, and disables powersave 
     * \param i2c Initialized I2CHandle configured at 400kHz or less
     */
    Result Init(I2CHandle i2c);

  private:
    /** Reads the data byte corresponding to the register address */
    Result ReadRegister(uint8_t addr, uint8_t *data);

    /** Writes the specified byte to the register at the specified address.*/
    Result WriteRegister(uint8_t addr, uint8_t val);

    I2CHandle i2c_;
    uint8_t   dev_addr_;
};

} // namespace daisy
#endif