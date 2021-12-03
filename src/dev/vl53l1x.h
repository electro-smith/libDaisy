#pragma once
#ifndef DSY_VL53L1X_H
#define DSY_VL53L1X_H

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** I2C Transport for VL53L1X */
class Vl53l1xI2CTransport
{
  public:
    Vl53l1xI2CTransport() {}
    ~Vl53l1xI2CTransport() {}

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

    void Write(uint8_t *data, uint16_t size)
    {
        // i2c_.TransmitBlocking(addr, data, size, 10);
    }

    void Read(uint8_t *data, uint16_t size)
    {
        // i2c_.ReceiveBlocking(addr, data, size, 10);
    }

  private:
    I2CHandle i2c_;
};


/** @brief Device support for VL53L1X Time of Flight Sensor
    @author beserge
    @date December 2021
*/
template <typename Transport>
class Vl53l1x
{
  public:
    Vl53l1x() {}
    ~Vl53l1x() {}

    struct Config
    {
        typename Transport::Config transport_config;
    };

    /** Initialize the VL53L1X device
        \param config Configuration settings
    */
    void Init(Config config)
    {
        config_ = config;

        transport_.Init(config_.transport_config);
    }

  private:
    Config    config_;
    Transport transport_;

}; // Class
/** @} */

using Vl53l1xI2C = Vl53l1x<Vl53l1xI2CTransport>;
} // namespace daisy
#endif
