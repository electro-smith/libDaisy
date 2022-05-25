#pragma once
#ifndef DSY_DPS310_H
#define DSY_DPS310_H

#define DPS310_I2CADDR_DEFAULT (0x77) ///< Default breakout addres

#define DPS310_PRSB2 0x00       ///< Highest byte of pressure data
#define DPS310_TMPB2 0x03       ///< Highest byte of temperature data
#define DPS310_PRSCFG 0x06      ///< Pressure configuration
#define DPS310_TMPCFG 0x07      ///< Temperature configuration
#define DPS310_MEASCFG 0x08     ///< Sensor configuration
#define DPS310_CFGREG 0x09      ///< Interrupt/FIFO configuration
#define DPS310_RESET 0x0C       ///< Soft reset
#define DPS310_PRODREVID 0x0D   ///< Register that contains the part ID
#define DPS310_TMPCOEFSRCE 0x28 ///< Temperature calibration src

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** I2C Transport for Dps310 */
class Dps310I2CTransport
{
  public:
    Dps310I2CTransport() {}
    ~Dps310I2CTransport() {}

    struct Config
    {
        I2CHandle::Config::Peripheral periph;
        I2CHandle::Config::Speed      speed;
        Pin                           scl;
        Pin                           sda;

        uint8_t address;

        Config()
        {
            address = DPS310_I2CADDR_DEFAULT;

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

/** SPI Transport for Dps310 */
class Dps310SpiTransport
{
  public:
    Dps310SpiTransport() {}
    ~Dps310SpiTransport() {}

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

/** \brief Device support for DPS310 Barometric Pressure and Altitude Sensor
    @author beserge
    @date January 2022
*/
template <typename Transport>
class Dps310
{
  public:
    Dps310() {}
    ~Dps310() {}

    int32_t oversample_scalefactor[8]
        = {524288, 1572864, 3670016, 7864320, 253952, 516096, 1040384, 2088960};

    /** The measurement rate ranges */
    enum dps310_rate_t
    {
        DPS310_1HZ,   ///< 1 Hz
        DPS310_2HZ,   ///< 2 Hz
        DPS310_4HZ,   ///< 4 Hz
        DPS310_8HZ,   ///< 8 Hz
        DPS310_16HZ,  ///< 16 Hz
        DPS310_32HZ,  ///< 32 Hz
        DPS310_64HZ,  ///< 64 Hz
        DPS310_128HZ, ///< 128 Hz
    };

    /** The  oversample rate ranges */
    enum dps310_oversample_t
    {
        DPS310_1SAMPLE,    ///< 1 Hz
        DPS310_2SAMPLES,   ///< 2 Hz
        DPS310_4SAMPLES,   ///< 4 Hz
        DPS310_8SAMPLES,   ///< 8 Hz
        DPS310_16SAMPLES,  ///< 16 Hz
        DPS310_32SAMPLES,  ///< 32 Hz
        DPS310_64SAMPLES,  ///< 64 Hz
        DPS310_128SAMPLES, ///< 128 Hz
    };

    /** The  oversample rate ranges */
    enum dps310_mode_t
    {
        DPS310_IDLE            = 0b000, ///< Stopped/idle
        DPS310_ONE_PRESSURE    = 0b001, ///< Take single pressure measurement
        DPS310_ONE_TEMPERATURE = 0b010, ///< Take single temperature measurement
        DPS310_CONT_PRESSURE   = 0b101, ///< Continuous pressure measurements
        DPS310_CONT_TEMP       = 0b110, ///< Continuous pressure measurements
        DPS310_CONT_PRESTEMP = 0b111, ///< Continuous temp+pressure measurements
    };

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

    /** Initialize the Dps310 device
        \param config Configuration settings
    */
    Result Init(Config config)
    {
        config_ = config;

        transport_.Init(config_.transport_config);

        // make sure we're talking to the right chip
        if(Read8(DPS310_PRODREVID) != 0x10)
        {
            // No DPS310 detected ... return false
            return ERR;
        }

        reset();
        _readCalibration();
        // default to high precision
        configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
        configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
        // continuous
        setMode(DPS310_CONT_PRESTEMP);
        // wait until we have at least one good measurement
        while(!temperatureAvailable() || !pressureAvailable())
        {
            System::Delay(10);
        }

        return GetTransportError();
    }


    /** Performs a software reset */
    void reset(void)
    {
        Write8(DPS310_RESET, 0x89);

        // Wait for a bit till its out of hardware reset
        System::Delay(10);

        while(!ReadBits(DPS310_MEASCFG, 1, 6))
        {
            System::Delay(1);
        }
    }

    static int32_t twosComplement(int32_t val, uint8_t bits)
    {
        if(val & ((uint32_t)1 << (bits - 1)))
        {
            val -= (uint32_t)1 << bits;
        }
        return val;
    }

    void _readCalibration(void)
    {
        // Wait till we're ready to read calibration
        while(!ReadBits(DPS310_MEASCFG, 1, 7))
        {
            System::Delay(1);
        }

        uint8_t coeffs[18];
        for(uint8_t addr = 0; addr < 18; addr++)
        {
            coeffs[addr] = Read8(0x10 + addr);
        }
        _c0 = ((uint16_t)coeffs[0] << 4) | (((uint16_t)coeffs[1] >> 4) & 0x0F);
        _c0 = twosComplement(_c0, 12);

        _c1 = twosComplement((((uint16_t)coeffs[1] & 0x0F) << 8) | coeffs[2],
                             12);

        _c00 = ((uint32_t)coeffs[3] << 12) | ((uint32_t)coeffs[4] << 4)
               | (((uint32_t)coeffs[5] >> 4) & 0x0F);
        _c00 = twosComplement(_c00, 20);

        _c10 = (((uint32_t)coeffs[5] & 0x0F) << 16) | ((uint32_t)coeffs[6] << 8)
               | (uint32_t)coeffs[7];
        _c10 = twosComplement(_c10, 20);

        _c01 = twosComplement(((uint16_t)coeffs[8] << 8) | (uint16_t)coeffs[9],
                              16);
        _c11 = twosComplement(
            ((uint16_t)coeffs[10] << 8) | (uint16_t)coeffs[11], 16);
        _c20 = twosComplement(
            ((uint16_t)coeffs[12] << 8) | (uint16_t)coeffs[13], 16);
        _c21 = twosComplement(
            ((uint16_t)coeffs[14] << 8) | (uint16_t)coeffs[15], 16);
        _c30 = twosComplement(
            ((uint16_t)coeffs[16] << 8) | (uint16_t)coeffs[17], 16);
    }


    /**  Whether new temperature data is available
        \return True if new data available to read
    */
    bool temperatureAvailable(void) { return ReadBits(DPS310_MEASCFG, 1, 5); }


    /**  Whether new pressure data is available
        \returns True if new data available to read
    */
    bool pressureAvailable(void) { return ReadBits(DPS310_MEASCFG, 1, 4); }


    /** Calculates the approximate altitude using barometric pressure and the supplied sea level hPa as a reference.
        \param seaLevelhPa The current hPa at sea level.
         \return The approximate altitude above sea level in meters.
    */
    float GetAltitude(float seaLevelhPa)
    {
        float altitude;

        Process();

        altitude = 44330 * (1.0 - pow((_pressure / 100) / seaLevelhPa, 0.1903));

        return altitude;
    }


    /**  Set the operational mode of the sensor (continuous or one-shot)
        \param mode can be DPS310_IDLE, one shot: DPS310_ONE_PRESSURE or
            DPS310_ONE_TEMPERATURE, continuous: DPS310_CONT_PRESSURE, DPS310_CONT_TEMP,
            DPS310_CONT_PRESTEMP
    */
    void setMode(dps310_mode_t mode) { WriteBits(DPS310_MEASCFG, mode, 3, 0); }


    /** Set the sample rate and oversampling averaging for pressure
        \param rate How many samples per second to take
        \param os How many oversamples to average
    */
    void configurePressure(dps310_rate_t rate, dps310_oversample_t os)
    {
        WriteBits(DPS310_PRSCFG, rate, 3, 4);
        WriteBits(DPS310_PRSCFG, os, 4, 0);

        if(os > DPS310_8SAMPLES)
        {
            WriteBits(DPS310_CFGREG, 1, 1, 2);
        }
        else
        {
            WriteBits(DPS310_CFGREG, 0, 1, 2);
        }

        pressure_scale = oversample_scalefactor[os];
    }


    /** Set the sample rate and oversampling averaging for temperature
        \param rate How many samples per second to take
        \param os How many oversamples to average
    */
    void configureTemperature(dps310_rate_t rate, dps310_oversample_t os)
    {
        WriteBits(DPS310_TMPCFG, rate, 3, 4);
        WriteBits(DPS310_TMPCFG, os, 4, 0);
        temp_scale = oversample_scalefactor[os];

        // Set shift bit if necessary
        if(os > DPS310_8SAMPLES)
        {
            WriteBits(DPS310_CFGREG, 1, 1, 3);
        }
        else
        {
            WriteBits(DPS310_CFGREG, 0, 1, 3);
        }

        // Find out what our calibration source is
        uint8_t read = ReadBits(DPS310_TMPCOEFSRCE, 1, 7);
        WriteBits(DPS310_TMPCFG, read, 1, 7);
    }


    /** Read the XYZ data from the sensor and store in the internal 
        raw_pressure, raw_temperature, _pressure and _temperature variables. 
    */
    void Process(void)
    {
        raw_temperature = twosComplement(Read24(DPS310_TMPB2), 24);
        raw_pressure    = twosComplement(Read24(DPS310_PRSB2), 24);

        _scaled_rawtemp = (float)raw_temperature / temp_scale;
        _temperature    = _scaled_rawtemp * _c1 + _c0 / 2.0;

        _pressure = (float)raw_pressure / pressure_scale;

        _pressure
            = (int32_t)_c00
              + _pressure
                    * ((int32_t)_c10
                       + _pressure
                             * ((int32_t)_c20 + _pressure * (int32_t)_c30))
              + _scaled_rawtemp
                    * ((int32_t)_c01
                       + _pressure
                             * ((int32_t)_c11 + _pressure * (int32_t)_c21));
    }

    /** Get last temperature reading
        \return temp in degrees Centigrade
    */
    float GetTemperature() { return _temperature; }

    /** Get the last pressure reading
        \return Pressure in hPa
    */
    float GetPressure() { return _pressure / 100; }

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


    uint8_t ReadBits(uint8_t reg, uint8_t bits, uint8_t shift)
    {
        uint8_t val = Read8(reg);
        val >>= shift;
        return val & ((1 << (bits)) - 1);
    }

    void WriteBits(uint8_t reg, uint8_t data, uint8_t bits, uint8_t shift)
    {
        uint8_t val = Read8(reg);

        // mask off the data before writing
        uint8_t mask = (1 << (bits)) - 1;
        data &= mask;

        mask <<= shift;
        val &= ~mask;         // remove the current data at that spot
        val |= data << shift; // and add in the new data

        Write8(reg, val);
    }

    /** Get and reset the transport error flag
        \return Whether the transport has errored since the last check
    */
    Result GetTransportError() { return transport_.GetError() ? ERR : OK; }

  private:
    Config    config_;
    Transport transport_;

    int16_t _c0, _c1, _c01, _c11, _c20, _c21, _c30;
    int32_t _c00, _c10;

    int32_t raw_pressure, raw_temperature;
    float   _temperature, _scaled_rawtemp, _pressure;
    int32_t temp_scale, pressure_scale;
};

/** @} */

using Dps310I2C = Dps310<Dps310I2CTransport>;
using Dps310Spi = Dps310<Dps310SpiTransport>;
} // namespace daisy
#endif