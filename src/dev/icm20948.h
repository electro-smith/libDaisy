#pragma once
#ifndef DSY_ICM20948_H
#define DSY_ICM20948_H

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** I2C Transport for Icm20948 */
class Icm20948I2CTransport
{
  public:
    Icm20948I2CTransport() {}
    ~Icm20948I2CTransport() {}

    struct Config
    {
        I2CHandle::Config::Peripheral periph;
        I2CHandle::Config::Speed      speed;
        Pin                           scl;
        Pin                           sda;

        uint8_t address;

        Config()
        {
            address = ;

            periph = I2CHandle::Config::Peripheral::I2C_1;
            speed  = I2CHandle::Config::Speed::I2C_400KHZ;

            scl = Pin(PORTB, 8);
            sda = Pin(PORTB, 9);
        }
    };

    inline void Init(Config config)
    {
        config_ = config;

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
        error_ |= I2CHandle::Result::OK
                  != i2c_.TransmitBlocking(config_.address, data, size, 10);
    }

    void Read(uint8_t *data, uint16_t size)
    {
        error_ |= I2CHandle::Result::OK
                  != i2c_.ReceiveBlocking(config_.address, data, size, 10);
    }

    /**  Writes an 8 bit value
        \param reg the register address to write to
        \param value the value to write to the register
    */
    void Write8(uint8_t reg, uint8_t value)
    {
        uint8_t buffer[2];

        buffer[0] = reg;
        buffer[1] = value;

        Write(buffer, 2);
    }

    /** Read from a reg address a defined number of bytes */
    void ReadReg(uint8_t reg, uint8_t *buff, uint8_t size)
    {
        Write(&reg, 1);
        Read(buff, size);
    }

    /**  Reads an 8 bit value
        \param reg the register address to read from
        \returns the 16 bit data value read from the device
    */
    uint8_t Read8(uint8_t reg)
    {
        uint8_t buffer;
        ReadReg(reg, &buffer, 1);
        return buffer;
    }

    /**  Reads a 16 bit value
        \param reg the register address to read from
        \returns the 16 bit data value read from the device
    */
    uint16_t Read16(uint8_t reg)
    {
        uint8_t buffer[2];
        ReadReg(reg, buffer, 2);

        return uint16_t(buffer[0]) << 8 | uint16_t(buffer[1]);
    }

    /**  Reads a 24 bit value
        \param reg the register address to read from
        \returns the 24 bit data value read from the device
    */
    uint32_t Read24(uint8_t reg)
    {
        uint8_t buffer[3];

        ReadReg(reg, buffer, 3);

        return uint32_t(buffer[0]) << 16 | uint32_t(buffer[1]) << 8
               | uint32_t(buffer[2]);
    }

    bool GetError()
    {
        bool tmp = error_;
        error_   = false;
        return tmp;
    }

  private:
    I2CHandle i2c_;
    Config    config_;

    // true if error has occured since last check
    bool error_;
};

/** SPI Transport for Icm20948 */
class Icm20948SpiTransport
{
  public:
    Icm20948SpiTransport() {}
    ~Icm20948SpiTransport() {}

    struct Config
    {
        SpiHandle::Config::Peripheral periph;
        Pin                           sclk;
        Pin                           miso;
        Pin                           mosi;
        Pin                           nss;

        Config()
        {
            periph = SpiHandle::Config::Peripheral::SPI_1;
            sclk   = Pin(PORTG, 11);
            miso   = Pin(PORTB, 4);
            mosi   = Pin(PORTB, 5);
            nss    = Pin(PORTG, 10);
        }
    };

    inline void Init(Config config)
    {
        SpiHandle::Config spi_conf;
        spi_conf.mode           = SpiHandle::Config::Mode::MASTER;
        spi_conf.direction      = SpiHandle::Config::Direction::TWO_LINES;
        spi_conf.clock_polarity = SpiHandle::Config::ClockPolarity::LOW;
        spi_conf.clock_phase    = SpiHandle::Config::ClockPhase::ONE_EDGE;
        spi_conf.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_2;
        spi_conf.nss            = SpiHandle::Config::NSS::SOFT;

        spi_conf.periph          = config.periph;
        spi_conf.pin_config.sclk = config.sclk;
        spi_conf.pin_config.miso = config.miso;
        spi_conf.pin_config.mosi = config.mosi;
        spi_conf.pin_config.nss  = config.nss;

        spi_.Init(spi_conf);
    }

    void Write(uint8_t *data, uint16_t size)
    {
        error_ |= SpiHandle::Result::OK != spi_.BlockingTransmit(data, size);
    }

    void Read(uint8_t *data, uint16_t size)
    {
        error_ |= SpiHandle::Result::OK != spi_.BlockingReceive(data, size, 10);
    }

    /**  Writes an 8 bit value
        \param reg the register address to write to
        \param value the value to write to the register
    */
    void Write8(uint8_t reg, uint8_t value)
    {
        uint8_t buffer[2];

        buffer[0] = reg & ~0x80;
        buffer[1] = value;

        Write(buffer, 2);
    }

    /** Read from a reg address a defined number of bytes */
    void ReadReg(uint8_t reg, uint8_t *buff, uint8_t size)
    {
        reg = uint8_t(reg | 0x80);
        Write(&reg, 1);
        Read(buff, size);
    }


    /**  Reads an 8 bit value
        \param reg the register address to read from
        \returns the data uint8_t read from the device
    */
    uint8_t Read8(uint8_t reg)
    {
        uint8_t buffer;
        ReadReg(reg, &buffer, 1);
        return buffer;
    }

    /**  Reads a 16 bit value over I2C or SPI
        \param reg the register address to read from
        \returns the 16 bit data value read from the device
    */
    uint16_t Read16(uint8_t reg)
    {
        uint8_t buffer[2];
        ReadReg(reg, buffer, 2);

        return uint16_t(buffer[0]) << 8 | uint16_t(buffer[1]);
    }

    /**  Reads a 24 bit value
        \param reg the register address to read from
        \returns the 24 bit data value read from the device
    */
    uint32_t Read24(uint8_t reg)
    {
        uint8_t buffer[3];

        ReadReg(reg, buffer, 3);

        return uint32_t(buffer[0]) << 16 | uint32_t(buffer[1]) << 8
               | uint32_t(buffer[2]);
    }

    bool GetError()
    {
        bool tmp = error_;
        error_   = false;
        return tmp;
    }

  private:
    SpiHandle spi_;
    bool      error_;
};

/** \brief Device support for ICM20948 TOF sensor
    @author beserge
    @date December 2021
*/
template <typename Transport>
class Icm20948
{
  public:
    Icm20948() {}
    ~Icm20948() {}

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

    /** Initialize the Icm20948 device
        \param config Configuration settings
    */
    Result Init(Config config)
    {
        config_ = config;

        transport_.Init(config_.transport_config);

        return GetTransportError();
    }

    /**  Writes an 8 bit value
        \param reg the register address to write to
        \param value the value to write to the register
    */
    void Write8(uint8_t reg, uint8_t value)
    {
        return transport_.Write8(reg, value);
    }

    /** Read from a reg address a defined number of bytes */
    void ReadReg(uint8_t reg, uint8_t *buff, uint8_t size)
    {
        return transport_.ReadReg(reg, buff, size);
    }

    /**  Reads an 8 bit value
        \param reg the register address to read from
        \returns the data uint8_t read from the device
    */
    uint8_t Read8(uint8_t reg) { return transport_.Read8(reg); }

    /**  Reads a 16 bit value over I2C or SPI
        \param reg the register address to read from
        \returns the 16 bit data value read from the device
    */
    uint16_t Read16(uint8_t reg) { return transport_.Read16(reg); }

    /**  Reads a 24 bit value
        \param reg the register address to read from
        \returns the 24 bit data value read from the device
    */
    uint32_t Read24(uint8_t reg) { return transport_.Read24(reg); }

    /**  Reads a signed 16 bit little endian value over I2C or SPI
        \param reg the register address to read from
        \returns the 16 bit data value read from the device
    */
    uint16_t Read16_LE(uint8_t reg)
    {
        uint16_t temp = Read16(reg);
        return (temp >> 8) | (temp << 8);
    }

    /**  Reads a signed 16 bit value over I2C or SPI
        \param reg the register address to read from
        \returns the 16 bit data value read from the device
    */
    int16_t ReadS16(uint8_t reg) { return (int16_t)Read16(reg); }

    /**  Reads a signed little endian 16 bit value over I2C or SPI
        \param reg the register address to read from
        \returns the 16 bit data value read from the device
    */
    int16_t ReadS16_LE(uint8_t reg) { return (int16_t)Read16_LE(reg); }

    /** Get and reset the transport error flag
        \return Whether the transport has errored since the last check
    */
    Result GetTransportError() { return transport_.GetError() ? ERR : OK; }

  private:
    Config    config_;
    Transport transport_;
};

/** @} */

using Icm20948I2C = Icm20948<Icm20948I2CTransport>;
using Icm20948Spi = Icm20948<Icm20948SpiTransport>;
} // namespace daisy
#endif