#pragma once
#ifndef DSY_CODEC_WM8731_H
#define DSY_CODEC_WM8731_H
#include "per/i2c.h"

namespace daisy
{
/** Device driver for Cirrus (Wolfsen) WM8731 Audio Codec
 ** 
 ** Currently only two-wire (I2C) interface format is supported, 
 ** and only a limited set of features are configurable:
 ** * Line inputs/outputs
 ** * audio format/word length
 ** * 48kHZ
 **
 ** Support for headphones, microphone, and full functionality 
 ** still needs to be added.
 **
 ** Use the Driver like this (this will be compatible with the Daisy Seed audio/sai config):
        I2CHandle::Config i2c_config;
        I2CHandle i2c1_handle;
        Wm8731::Config codec_cfg;
        Wm8731 codec;
        i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
        i2c_config.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
        i2c_config.pin_config.scl = {DSY_GPIOB, 6};
        i2c_config.pin_config.sda = {DSY_GPIOB, 9};
        i2c1_handle.Init(i2c_config);
        codec_cfg.Defaults(); // MCU is master, 24-bit,  MSB LJ
        codec.Init(codec_cfg, i2c1_handle); 
 ** 
 ***/
class Wm8731
{
  public:
    /** Return values for WM8731 Functions */
    enum class Result
    {
        OK,
        ERR,
    };

    /** Configuration struct for use in initializing the device. 
     ** For now, only 48kHz is supported. 
     ** USB Mode is also not yet supported. */
    struct Config
    {
        /** Sets the communication format used */
        enum class Format
        {
            MSB_FIRST_RJ = 0x00,
            MSB_FIRST_LJ = 0x01,
            I2S          = 0x02,
            DSP          = 0x03,
        };

        /** Defines the size of a sample in bits
         ** This is for communication only, the device
         ** processes audio at 24-bits, and the strips/pads bits
         ** to send to the processor. */
        enum class WordLength
        {
            BITS_16 = (0x00 << 2),
            BITS_20 = (0x01 << 2),
            BITS_24 = (0x02 << 2),
            BITS_32 = (0x03 << 2),
        };

        /** Sets the device to slave mode if true, and master mode if false. */
        bool mcu_is_master;

        /** Sets whether the left/right channels are swapped or not. */
        bool lr_swap;

        /** Set true if tied high, and false if tied low. 
         ** determines the I2C address for communicating 
         ** with the device */
        bool csb_pin_state;

        Format     fmt;
        WordLength wl;

        /** Sets the following config:
         ** MCU is master = true 
         ** L/R Swap = false
         ** CSB Pin state = false
         ** Format = MSB First LJ 
         ** WordLength = 24-bit  */
        void Defaults()
        {
            mcu_is_master = true;
            lr_swap       = false;
            csb_pin_state = false;
            fmt           = Format::MSB_FIRST_LJ;
            wl            = WordLength::BITS_24;
        }
    };

    Wm8731() {}
    ~Wm8731() {}

    /** Initializes the WM8731 device */
    Result Init(const Config &config, I2CHandle i2c);

  private:
    I2CHandle i2c_;
    Config    cfg_;
    Result    WriteControlRegister(uint8_t addr, uint16_t data);
    uint8_t   dev_addr_;
};

} // namespace daisy


#endif
