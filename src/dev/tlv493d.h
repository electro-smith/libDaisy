#pragma once
#ifndef DSY_TLV493D_H
#define DSY_TLV493D_H

#define TLV493D_ADDRESS1 0x5E
#define TLV493D_ADDRESS2 0x1F

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** I2C Transport for TLV493D */
class Tlv493dI2CTransport
{
  public:
    Tlv493dI2CTransport() {}
    ~Tlv493dI2CTransport() {}

    struct Config
    {
        I2CHandle::Config::Peripheral periph;
        I2CHandle::Config::Speed      speed;
        dsy_gpio_pin                  scl;
        dsy_gpio_pin                  sda;

        Config()
        {
            periph = I2CHandle::Config::Peripheral::I2C_1;
            speed  = I2CHandle::Config::Speed::I2C_400KHZ;

            scl = {DSY_GPIOB, 8};
            sda = {DSY_GPIOB, 9};
        }
    };

    inline void Init(Config config)
    {
        I2CHandle::Config i2c_config;
        i2c_config.mode   = I2CHandle::Config::Mode::I2C_MASTER;
        i2c_config.periph = config.periph;
        i2c_config.speed  = config.speed;

        i2c_config.pin_config.scl = config.scl;
        i2c_config.pin_config.sda = config.sda;

        i2c_.Init(i2c_config);
    }

    void Write(uint8_t* data, uint16_t size)
    {
        i2c_.TransmitBlocking(TLV493D_ADDRESS1, data, size, 10);
    }

    void Read(uint8_t* data, uint16_t size)
    {
        i2c_.ReceiveBlocking(TLV493D_ADDRESS1, data, size, 10);
    }

  private:
    I2CHandle i2c_;
};


/** @brief Device support for TLV493D Magnetic Sensor
    @author beserge
    @date December 2021
*/
template <typename Transport>
class Tlv493d
{
  public:
    Tlv493d() {}
    ~Tlv493d() {}

    struct Config
    {
        typename Transport::Config transport_config;
        uint8_t address;
        Config(){
            address = TLV493D_ADDRESS1;
        }
    };

    /** Initialize the TLV493D device
        \param config Configuration settings
    */
    void Init(Config config)
    {
        config_ = config;

        transport_.Init(config_.transport_config);

        System::Delay(40); // 40ms startup delay

        ResetSensor(config_.address);

        // get all register data from sensor
        // tlv493d::readOut(&mInterface);
        // copy factory settings to write registers
        // setRegBits(tlv493d::W_RES1, getRegBits(tlv493d::R_RES1));
        // setRegBits(tlv493d::W_RES2, getRegBits(tlv493d::R_RES2));
        // setRegBits(tlv493d::W_RES3, getRegBits(tlv493d::R_RES3));
        // enable parity detection
        // setRegBits(tlv493d::W_PARITY_EN, 1);
        // config sensor to lowpower mode
        // also contains parity calculation and writeout to sensor
        // setAccessMode(TLV493D_DEFAULTMODE);
    }


  private:
    Config    config_;
    Transport transport_;


    // internal function called by begin()
    void ResetSensor(uint8_t adr)
    {
        uint8_t data[2];
        data[0] = 0x00;
        if(adr == TLV493D_ADDRESS1)
        {
            // if the sensor shall be initialized with i2c address 0x1F
            data[1] = 0xFF;
        }
        else
        {
            // if the sensor shall be initialized with address 0x5E
            data[1] = 0x00;
        }

        transport_.Write(data, 2);
    }

};

/** @} */

using Tlv493dI2C = Tlv493d<Tlv493dI2CTransport>;
} // namespace daisy
#endif
