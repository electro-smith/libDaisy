#pragma once
#ifndef DSY_BMP390_H
#define DSY_BMP390_H

#define BMP3_IIR_FILTER_DISABLE UINT8_C(0x00)
#define BMP3_ODR_25_HZ UINT8_C(0x03)
#define BMP390_CHIP_ID UINT8_C(0x60)
#define BMP390_SLAVE_ADD UINT8_C(0xEC)

#define BMP3_REG_OSR UINT8_C(0x1C)
#define BMP3_REG_ODR UINT8_C(0x1D)
#define BMP3_REG_IIR UINT8_C(0x1F)

namespace daisy
{
/** @addtogroup external 
    @{ 
*/


class Bmp390I2CTransport
{
  public:
    Bmp390I2CTransport() {}
    ~Bmp390I2CTransport() {}

    struct Config
    {
        I2CHandle::Config::Peripheral periph;
        I2CHandle::Config::Speed speed;
        dsy_gpio_pin scl;
        dsy_gpio_pin sda;
        
        Config()
        {
            periph = I2CHandle::Config::Peripheral::I2C_1;
            speed = I2CHandle::Config::Speed::I2C_400KHZ;
        
            scl = {DSY_GPIOB, 8};
            sda = {DSY_GPIOB, 9};
        }
    };

    inline void Init(Config config)
    {
        I2CHandle::Config i2c_config;
        i2c_config.mode = I2CHandle::Config::Mode::I2C_MASTER;
        i2c_config.periph = config.periph;
        i2c_config.speed = config.speed;

        i2c_config.pin_config.scl = config.scl;
        i2c_config.pin_config.sda = config.sda;

        i2c_.Init(i2c_config);
    }

    void Write(uint8_t* data, uint16_t size){
        i2c_.TransmitBlocking(BMP390_CHIP_ID, data, size, 10);
    }

    void Read(uint8_t* data, uint16_t size){
        i2c_.ReceiveBlocking(BMP390_CHIP_ID, data, size, 10);
    }

  private:
  I2CHandle i2c_;
};

class Bmp390SpiTransport{
  public:
    Bmp390SpiTransport() {}
    ~Bmp390SpiTransport() {}

    struct Config
    {
        SpiHandle::Config::Peripheral periph;
        Config()
        {
        }
    };

    inline void Init(Config config)
    {
        SpiHandle::Config spi_conf;
        spi_conf.mode = SpiHandle::Config::Mode::MASTER;
        spi_conf.direction = SpiHandle::Config::Direction::TWO_LINES;
        spi_conf.clock_polarity = SpiHandle::Config::ClockPolarity::LOW;
        spi_conf.clock_phase = SpiHandle::Config::ClockPhase::ONE_EDGE;
        spi_conf.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_2;

        spi_.Init(spi_conf);
    }

  private:
  SpiHandle spi_;
};

/** 
    @brief 
    @author beserge
    @date November 2021
*/
template <typename Transport>
class Bmp390
{
  public:
    Bmp390() {}
    ~Bmp390() {}

    struct Config
    {
        typename Transport::Config transport_config;
    };

    enum Result{
        OK,
        ERR
    };

    /** */
    void Init(Config config)
    {
        config_ = config;

        transport_.Init(config_.transport_config);

        SetOversampling(0, 0); // no oversampling for temp or pressure
        SetOutputDataRate(3); // 25 Hz
        SetIIRFilterCoeff(0); // disabled
    }

    /** Subdivision factor for pressure and temperature measurements is
    2^value. Allowed values are 0...17. Other values are saturated at 17. */
    void SetOutputDataRate(uint8_t rate){
        if(rate > 17)
            rate = 17;

        WriteToReg(BMP3_REG_ODR, rate);
    }

    void SetIIRFilterCoeff(uint8_t coeff){
        if(coeff > 7)
            coeff = 7;

        WriteToReg(BMP3_REG_IIR, coeff);
    }

    /** (0,5) = {1, 2, 4, 8, 16, 32} */
    void SetOversampling(uint8_t temp, uint8_t press){
        if(temp > 5)
            temp = 5;

        if(press > 5)
            press = 5;

        uint8_t val = temp | press << 3;

        WriteToReg(BMP3_REG_OSR, val);
    }

    void WriteToReg(uint8_t reg, uint8_t data){
        uint8_t write_buff[3];

        write_buff[0] = BMP390_SLAVE_ADD | ((uint8_t)sdo_state_ << 1); // slave address | SDO state (111011X0)
        write_buff[1] = reg; // register address
        write_buff[2] = data; // value to write

        transport_.Write(write_buff, 3);
    }

  private: 
    bool sdo_state_;
    Config                     config_;
    Transport                  transport_;
};

/** @} */

using Bmp390I2C = Bmp390<Bmp390I2CTransport>;
using Bmp390Spi = Bmp390<Bmp390SpiTransport>;
} // namespace daisy
#endif
