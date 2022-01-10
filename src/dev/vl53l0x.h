#pragma once
#ifndef DSY_VL53L0X_H
#define DSY_VL53L0X_H

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** I2C Transport for VL53L1X */
class Vl53l0xI2CTransport
{
  public:
    Vl53l0xI2CTransport() {}
    ~Vl53l0xI2CTransport() {}

    struct Config
    {
        I2CHandle::Config::Peripheral periph;
        I2CHandle::Config::Speed      speed;
        Pin                           scl;
        Pin                           sda;

        uint8_t dev_addr;

        Config()
        {
            periph = I2CHandle::Config::Peripheral::I2C_1;
            speed  = I2CHandle::Config::Speed::I2C_400KHZ;

            scl = Pin(PORTB, 8);
            sda = Pin(PORTB, 9);

            dev_addr = VL53L0X_I2C_ADDR;
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

        error_ = false;

        error_ |= I2CHandle::Result::OK != i2c_.Init(i2c_config);
    }

    void Write(uint8_t *data, uint16_t size)
    {
        I2CHandle::Result res
            = i2c_.TransmitBlocking(dev_addr_, data, size, 10);
        error_ |= (res != I2CHandle::\result::OK);
    }

    void Read(uint8_t *data, uint16_t size)
    {
        I2CHandle::Result res = i2c_.ReceiveBlocking(dev_addr_, data, size, 10);
        error_ |= (res != I2CHandle::Result::OK);
    }

    void Write8(uint8_t reg, uint8_t val)
    {
        uint8_t buff[2] = {reg, val};
        Write(buff, 2);
    }

    uint8_t Read8(uint8_t reg)
    {
        uint8_t ret;

        Write(reg);
        Read(ret);
        return ret;
    }
    /** \return true if error occured since last check */
    bool GetError()
    {
        bool tmp = error_;
        error_   = false;
        return error_;
    }

  private:
    I2CHandle i2c_;
    uint8_t   dev_addr_;
    bool      error_;
};


/** Device support for VL53L0X Time of Flight Sensor
    \author beserge
    \date December 2021
*/
template <typename Transport>
class Vl53l0x
{
  public:
    Vl53l0x() {}
    ~Vl53l0x() {}

    struct Config
    {
        typename Transport::Config transport_config;

        Config() {}
    };

    enum Result
    {
        OK = 0,
        ERR
    };

    /** Initialize the VL53L0X device
        \param config Configuration settings
    */
    Result Init(Config config)
    {
        uint32_t refSpadCount;
        uint8_t  isApertureSpads;
        uint8_t  VhvSettings;
        uint8_t  PhaseCal;

        config_ = config;
        transport_.Init(config_.transport_config);

        VL53L0X_DataInit(); // Data initialization

        VL53L0X_GetDeviceInfo(&DeviceInfo);

        if((DeviceInfo.ProductRevisionMajor != 1)
           || (DeviceInfo.ProductRevisionMinor != 1))
        {
            return ERR;
        }

        VL53L0X_StaticInit(); // Device Initialization

        // Device Initialization
        VL53L0X_PerformRefSpadManagement(&refSpadCount, &isApertureSpads);

        // Device Initialization
        Status = VL53L0X_PerformRefCalibration(&VhvSettings, &PhaseCal);

        // no need to do this when we use VL53L0X_PerformSingleRangingMeasurement
        // Setup in single ranging mode
        VL53L0X_SetDeviceMode(VL53L0X_DEVICEMODE_SINGLE_RANGING);

        // call off to the config function to do the last part of configuration.
        ConfigSensor(vl_config);

        return GetTransportError();
    }

    uint8_t Read8(uint8_t reg) { return transport_.Read8(reg); }

    void Write8(uint8_t reg, uint8_t val) { transport_.Write8(reg, val); }

    void Update8(uint8_t reg, uint8_t AndData, uint8_t OrData)
    {
        uint8_t data = Read8(reg);
        data         = (data & AndData) | OrData;
        Write8(reg, data);
    }

    Result GetTransportError() { return transport_.GetError() ? ERR : OK; }

}; // namespace daisy
/** @} */

using Vl53l0xI2C = Vl53l0x<Vl53l0xI2CTransport>;
} // namespace daisy
#endif