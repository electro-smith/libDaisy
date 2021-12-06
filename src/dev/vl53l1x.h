#pragma once
#ifndef DSY_VL53L1X_H
#define DSY_VL53L1X_H

#define DEFAULT_DEV_ADDR 0x52

#define VL53L1X_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND 0x0008
#define GPIO_HV_MUX__CTRL 0x0030
#define GPIO__TIO_HV_STATUS 0x0031
#define SYSTEM__INTERRUPT_CLEAR 0x0086
#define SYSTEM__MODE_START 0x0087

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

        uint8_t dev_addr;

        Config()
        {
            periph = I2CHandle::Config::Peripheral::I2C_1;
            speed  = I2CHandle::Config::Speed::I2C_400KHZ;

            scl = {DSY_GPIOB, 8};
            sda = {DSY_GPIOB, 9};

            dev_addr = DEFAULT_DEV_ADDR;
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
        i2c_.TransmitBlocking(dev_addr_, data, size, 10);
    }

    void Read(uint8_t *data, uint16_t size)
    {
        // dev_addr_ may need >> 1...
        i2c_.ReceiveBlocking(dev_addr_, data, size, 10);
    }

  private:
    I2CHandle i2c_;
    uint8_t   dev_addr_;
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
        dsy_gpio_pin
                                   xShut; // active low. Not sure if we need it but it's nice to have
        typename Transport::Config transport_config;
    };

    /** Initialize the VL53L1X device
        \param config Configuration settings
    */
    void Init(Config config)
    {
        config_ = config;

        dsy_gpio gpio_conf;
        gpio_conf.pin  = config_.xShut;
        gpio_conf.mode = DSY_GPIO_MODE_OUTPUT_PP;
        gpio_conf.pull = DSY_GPIO_NOPULL;
        dsy_gpio_init(&gpio_conf);
        dsy_gpio_write(config_.xShut, true);

        transport_.Init(config_.transport_config);

        // initialize the device...
        uint8_t Addr = 0x00, tmp = 0;

        for(Addr = 0x2D; Addr <= 0x87; Addr++)
        {
            Write8(Addr, VL51L1X_DEFAULT_CONFIGURATION[Addr - 0x2D]);
        }

        StartRanging();

        while(tmp == 0)
        {
            CheckForDataReady(&tmp);
        }

        ClearInterrupt();
        StopRanging();

        // two bounds VHV
        Write8(VL53L1X_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND, 0x09);

        // start VHV from the previous temperature
        Write8(0x0B, 0);
    }

    void Write8(uint8_t devaddr, uint16_t index, uint8_t data)
    {
        uint8_t buff[] = {index >> 8, index & 0xFF, data};
        transport_.Write(buff, 3);
    }

    void StartRanging()
    {
        Write8(SYSTEM__MODE_START, 0x40); /* Enable VL53L1X */
    }

    void StopRanging()
    {
        Write8(SYSTEM__MODE_START, 0x00); /* Disable VL53L1X */
    }

    bool CheckForDataReady()
    {
        uint8_t Temp;
        uint8_t IntPol;

        GetInterruptPolarity(&IntPol);
        Temp = Read8(GPIO__TIO_HV_STATUS);

        return (Temp & 1) == IntPol;
    }

    uint8_t GetInterruptPolarity()
    {
        uint8_t Temp;

        Temp = Read8(GPIO_HV_MUX__CTRL);
        Temp &= 0x10;
        return !(Temp >> 4);
    }

    uint8_t Read8(uint16_t index)
    {
        // read a byte
        uint8_t buff[] = {index >> 8, index & 0xff};
        uint8_t ret;

        transport_.Write(buff, 2);
        transport_.Read(ret, 1);

        return ret;
    }

    void ClearInterrupt() { Write8(SYSTEM__INTERRUPT_CLEAR, 0x01); }

  private:
    Config    config_;
    Transport transport_;

    // yikes...
    const uint8_t VL51L1X_DEFAULT_CONFIGURATION[] = {
        0x00, /* 0x2d : set bit 2 and 5 to 1 for fast plus mode (1MHz I2C), else don't touch */
        0x00, /* 0x2e : bit 0 if I2C pulled up at 1.8V, else set bit 0 to 1 (pull up at AVDD) */
        0x00, /* 0x2f : bit 0 if GPIO pulled up at 1.8V, else set bit 0 to 1 (pull up at AVDD) */
        0x01, /* 0x30 : set bit 4 to 0 for active high interrupt and 1 for active low (bits 3:0 must be 0x1), use SetInterruptPolarity() */
        0x02, /* 0x31 : bit 1 = interrupt depending on the polarity, use CheckForDataReady() */
        0x00, /* 0x32 : not user-modifiable */
        0x02, /* 0x33 : not user-modifiable */
        0x08, /* 0x34 : not user-modifiable */
        0x00, /* 0x35 : not user-modifiable */
        0x08, /* 0x36 : not user-modifiable */
        0x10, /* 0x37 : not user-modifiable */
        0x01, /* 0x38 : not user-modifiable */
        0x01, /* 0x39 : not user-modifiable */
        0x00, /* 0x3a : not user-modifiable */
        0x00, /* 0x3b : not user-modifiable */
        0x00, /* 0x3c : not user-modifiable */
        0x00, /* 0x3d : not user-modifiable */
        0xff, /* 0x3e : not user-modifiable */
        0x00, /* 0x3f : not user-modifiable */
        0x0F, /* 0x40 : not user-modifiable */
        0x00, /* 0x41 : not user-modifiable */
        0x00, /* 0x42 : not user-modifiable */
        0x00, /* 0x43 : not user-modifiable */
        0x00, /* 0x44 : not user-modifiable */
        0x00, /* 0x45 : not user-modifiable */
        0x20, /* 0x46 : interrupt configuration 0->level low detection, 1-> level high, 2-> Out of window, 3->In window, 0x20-> New sample ready , TBC */
        0x0b, /* 0x47 : not user-modifiable */
        0x00, /* 0x48 : not user-modifiable */
        0x00, /* 0x49 : not user-modifiable */
        0x02, /* 0x4a : not user-modifiable */
        0x0a, /* 0x4b : not user-modifiable */
        0x21, /* 0x4c : not user-modifiable */
        0x00, /* 0x4d : not user-modifiable */
        0x00, /* 0x4e : not user-modifiable */
        0x05, /* 0x4f : not user-modifiable */
        0x00, /* 0x50 : not user-modifiable */
        0x00, /* 0x51 : not user-modifiable */
        0x00, /* 0x52 : not user-modifiable */
        0x00, /* 0x53 : not user-modifiable */
        0xc8, /* 0x54 : not user-modifiable */
        0x00, /* 0x55 : not user-modifiable */
        0x00, /* 0x56 : not user-modifiable */
        0x38, /* 0x57 : not user-modifiable */
        0xff, /* 0x58 : not user-modifiable */
        0x01, /* 0x59 : not user-modifiable */
        0x00, /* 0x5a : not user-modifiable */
        0x08, /* 0x5b : not user-modifiable */
        0x00, /* 0x5c : not user-modifiable */
        0x00, /* 0x5d : not user-modifiable */
        0x01, /* 0x5e : not user-modifiable */
        0xcc, /* 0x5f : not user-modifiable */
        0x0f, /* 0x60 : not user-modifiable */
        0x01, /* 0x61 : not user-modifiable */
        0xf1, /* 0x62 : not user-modifiable */
        0x0d, /* 0x63 : not user-modifiable */
        0x01, /* 0x64 : Sigma threshold MSB (mm in 14.2 format for MSB+LSB), use SetSigmaThreshold(), default value 90 mm  */
        0x68, /* 0x65 : Sigma threshold LSB */
        0x00, /* 0x66 : Min count Rate MSB (MCPS in 9.7 format for MSB+LSB), use SetSignalThreshold() */
        0x80, /* 0x67 : Min count Rate LSB */
        0x08, /* 0x68 : not user-modifiable */
        0xb8, /* 0x69 : not user-modifiable */
        0x00, /* 0x6a : not user-modifiable */
        0x00, /* 0x6b : not user-modifiable */
        0x00, /* 0x6c : Intermeasurement period MSB, 32 bits register, use SetIntermeasurementInMs() */
        0x00, /* 0x6d : Intermeasurement period */
        0x0f, /* 0x6e : Intermeasurement period */
        0x89, /* 0x6f : Intermeasurement period LSB */
        0x00, /* 0x70 : not user-modifiable */
        0x00, /* 0x71 : not user-modifiable */
        0x00, /* 0x72 : distance threshold high MSB (in mm, MSB+LSB), use SetD:tanceThreshold() */
        0x00, /* 0x73 : distance threshold high LSB */
        0x00, /* 0x74 : distance threshold low MSB ( in mm, MSB+LSB), use SetD:tanceThreshold() */
        0x00, /* 0x75 : distance threshold low LSB */
        0x00, /* 0x76 : not user-modifiable */
        0x01, /* 0x77 : not user-modifiable */
        0x0f, /* 0x78 : not user-modifiable */
        0x0d, /* 0x79 : not user-modifiable */
        0x0e, /* 0x7a : not user-modifiable */
        0x0e, /* 0x7b : not user-modifiable */
        0x00, /* 0x7c : not user-modifiable */
        0x00, /* 0x7d : not user-modifiable */
        0x02, /* 0x7e : not user-modifiable */
        0xc7, /* 0x7f : ROI center, use SetROI() */
        0xff, /* 0x80 : XY ROI (X=Width, Y=Height), use SetROI() */
        0x9B, /* 0x81 : not user-modifiable */
        0x00, /* 0x82 : not user-modifiable */
        0x00, /* 0x83 : not user-modifiable */
        0x00, /* 0x84 : not user-modifiable */
        0x01, /* 0x85 : not user-modifiable */
        0x00, /* 0x86 : clear interrupt, use ClearInterrupt() */
        0x00 /* 0x87 : start ranging, use StartRanging() or StopRanging(), If you want an automatic start after VL53L1X_init() call, put 0x40 in location 0x87 */
    };

}; // Class
/** @} */

using Vl53l1xI2C = Vl53l1x<Vl53l1xI2CTransport>;
} // namespace daisy
#endif
