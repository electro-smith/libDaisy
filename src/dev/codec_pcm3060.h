#pragma once
#ifndef DSY_CODEC_PCM3060_H
#define DSY_CODEC_PCM3060_H

namespace daisy
{
/**
 * @brief Driver for the PCM3060 Codec.
 * @addtogroup codec
 */
class Pcm3060
{
  public:
    Pcm3060() {}
    ~Pcm3060() {}
    void Init(I2CHandle i2c);

  private:
    I2CHandle i2c_;
    uint8_t   dev_addr_;
    uint8_t   ReadRegister(uint8_t addr);
    void      WriteRegister(uint8_t addr, uint8_t val);
};

} // namespace daisy
#endif