#pragma once
#ifndef DSY_BME280_H
#define DSY_BME280_H

/** Primary I2C Address */
#define BME280_ADDRESS (0x77)

/** Alternate I2C address */
#define BME280_ADDRESS_ALTERNATE (0x76)

/* Register addresses */
#define BME280_REGISTER_DIG_T1 0x88
#define BME280_REGISTER_DIG_T2 0x8A
#define BME280_REGISTER_DIG_T3 0x8C
#define BME280_REGISTER_DIG_P1 0x8E
#define BME280_REGISTER_DIG_P2 0x90
#define BME280_REGISTER_DIG_P3 0x92
#define BME280_REGISTER_DIG_P4 0x94
#define BME280_REGISTER_DIG_P5 0x96
#define BME280_REGISTER_DIG_P6 0x98
#define BME280_REGISTER_DIG_P7 0x9A
#define BME280_REGISTER_DIG_P8 0x9C
#define BME280_REGISTER_DIG_P9 0x9E
#define BME280_REGISTER_DIG_H1 0xA1
#define BME280_REGISTER_DIG_H2 0xE1
#define BME280_REGISTER_DIG_H3 0xE3
#define BME280_REGISTER_DIG_H4 0xE4
#define BME280_REGISTER_DIG_H5 0xE5
#define BME280_REGISTER_DIG_H6 0xE7

#define BME280_REGISTER_CHIPID 0xD0
#define BME280_REGISTER_VERSION 0xD1
#define BME280_REGISTER_SOFTRESET 0xE0

#define BME280_REGISTER_CAL26 0xE1 // R calibration stored in 0xE1-0xF0
#define BME280_REGISTER_CONTROLHUMID 0xF2
#define BME280_REGISTER_STATUS 0XF3
#define BME280_REGISTER_CONTROL 0xF4
#define BME280_REGISTER_CONFIG 0xF5
#define BME280_REGISTER_PRESSUREDATA 0xF7
#define BME280_REGISTER_TEMPDATA 0xFA
#define BME280_REGISTER_HUMIDDATA 0xFD

#define SAMPLING_MODE_DEFAULT MODE_NORMAL
#define SAMPLING_TEMP_DEFAULT SAMPLING_X16
#define SAMPLING_PRESS_DEFAULT SAMPLING_X16
#define SAMPLING_HUM_DEFAULT SAMPLING_X16
#define SAMPLING_FILTER_DEFAULT FILTER_OFF
#define SAMPLING_DURATION_DEFAULT STANDBY_MS_0_5

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** I2C Transport for Bme280 */
class Bme280I2CTransport
{
  public:
    Bme280I2CTransport() {}
    ~Bme280I2CTransport() {}

    struct Config
    {
        I2CHandle::Config::Peripheral periph;
        I2CHandle::Config::Speed      speed;
        dsy_gpio_pin                  scl;
        dsy_gpio_pin                  sda;

        uint8_t address;

        Config()
        {
            address = BME280_ADDRESS;

            periph = I2CHandle::Config::Peripheral::I2C_1;
            speed  = I2CHandle::Config::Speed::I2C_400KHZ;

            scl = {DSY_GPIOB, 8};
            sda = {DSY_GPIOB, 9};
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

/** SPI Transport for Bme280 */
class Bme280SpiTransport
{
  public:
    Bme280SpiTransport() {}
    ~Bme280SpiTransport() {}

    struct Config
    {
        SpiHandle::Config::Peripheral periph;
        dsy_gpio_pin                  sclk;
        dsy_gpio_pin                  miso;
        dsy_gpio_pin                  mosi;
        dsy_gpio_pin                  nss;

        Config()
        {
            periph = SpiHandle::Config::Peripheral::SPI_1;
            sclk   = {DSY_GPIOG, 11};
            miso   = {DSY_GPIOB, 4};
            mosi   = {DSY_GPIOB, 5};
            nss    = {DSY_GPIOG, 10};
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

/** \brief Device support for BME280 Humidity Pressure Sensor
    @author beserge
    @date December 2021
*/
template <typename Transport>
class Bme280
{
  public:
    Bme280() {}
    ~Bme280() {}


    /** Sampling rates */
    enum sensor_sampling
    {
        SAMPLING_NONE = 0b000,
        SAMPLING_X1   = 0b001,
        SAMPLING_X2   = 0b010,
        SAMPLING_X4   = 0b011,
        SAMPLING_X8   = 0b100,
        SAMPLING_X16  = 0b101
    };

    /** Power Modes */
    enum sensor_mode
    {
        MODE_SLEEP  = 0b00,
        MODE_FORCED = 0b01,
        MODE_NORMAL = 0b11
    };

    /** Filter Values */
    enum sensor_filter
    {
        FILTER_OFF = 0b000,
        FILTER_X2  = 0b001,
        FILTER_X4  = 0b010,
        FILTER_X8  = 0b011,
        FILTER_X16 = 0b100
    };

    /** standby duration in ms */
    enum standby_duration
    {
        STANDBY_MS_0_5  = 0b000,
        STANDBY_MS_10   = 0b110,
        STANDBY_MS_20   = 0b111,
        STANDBY_MS_62_5 = 0b001,
        STANDBY_MS_125  = 0b010,
        STANDBY_MS_250  = 0b011,
        STANDBY_MS_500  = 0b100,
        STANDBY_MS_1000 = 0b101
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

    /** Initialize the Bme280 device
        \param config Configuration settings
    */
    Result Init(Config config)
    {
        config_ = config;

        transport_.Init(config_.transport_config);

        // reset the device using soft-reset
        // this makes sure the IIR is off, etc.
        Write8(BME280_REGISTER_SOFTRESET, 0xB6);

        // wait for chip to wake up.
        System::Delay(10);

        // if chip is still reading calibration, delay
        while(isReadingCalibration())
            System::Delay(10);

        ReadCoefficients(); // read trimming parameters, see DS 4.2.2

        // use defaults
        SetSampling(SAMPLING_MODE_DEFAULT,
                    SAMPLING_TEMP_DEFAULT,
                    SAMPLING_PRESS_DEFAULT,
                    SAMPLING_HUM_DEFAULT,
                    SAMPLING_FILTER_DEFAULT,
                    SAMPLING_DURATION_DEFAULT);

        System::Delay(100);

        return GetTransportError();
    }

    /**  setup sensor with given parameters / settings

        This is simply a overload to the normal begin()-function, so SPI users
        don't get confused about the library requiring an address.
        \param mode the power mode to use for the sensor
        \param tempSampling the temp samping rate to use
        \param pressSampling the pressure sampling rate to use
        \param humSampling the humidity sampling rate to use
        \param filter the filter mode to use
        \param duration the standby duration to use
    */
    void SetSampling(sensor_mode      mode,
                     sensor_sampling  tempSampling,
                     sensor_sampling  pressSampling,
                     sensor_sampling  humSampling,
                     sensor_filter    filter,
                     standby_duration duration)
    {
        _measReg.mode   = mode;
        _measReg.osrs_t = tempSampling;
        _measReg.osrs_p = pressSampling;

        _humReg.osrs_h      = humSampling;
        _configReg.filter   = filter;
        _configReg.t_sb     = duration;
        _configReg.spi3w_en = 0;

        // making sure sensor is in sleep mode before setting configuration
        // as it otherwise may be ignored
        Write8(BME280_REGISTER_CONTROL, MODE_SLEEP);

        // you must make sure to also set REGISTER_CONTROL after setting the
        // CONTROLHUMID register, otherwise the values won't be applied (see
        // DS 5.4.3)
        Write8(BME280_REGISTER_CONTROLHUMID, _humReg.get());
        Write8(BME280_REGISTER_CONFIG, _configReg.get());
        Write8(BME280_REGISTER_CONTROL, _measReg.get());
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

    /** Take a new measurement (only possible in forced mode)
        \returns true in case of success else false
    */
    bool TakeForcedMeasurement()
    {
        bool return_value = false;
        // If we are in forced mode, the BME sensor goes back to sleep after each
        // measurement and we need to set it to forced mode once at this point, so
        // it will take the next measurement and then return to sleep again.
        // In normal mode simply does new measurements periodically.
        if(_measReg.mode == MODE_FORCED)
        {
            return_value = true;
            // set to forced mode, i.e. "take next measurement"
            Write8(BME280_REGISTER_CONTROL, _measReg.get());
            // Store current time to measure the timeout
            uint32_t timeout_start = System::GetNow();
            // wait until measurement has been completed, otherwise we would read the
            // the values from the last measurement or the timeout occurred after 2 sec.
            while(Read8(BME280_REGISTER_STATUS) & 0x08)
            {
                // In case of a timeout, stop the while loop
                if((System::GetNow() - timeout_start) > 2000)
                {
                    return_value = false;
                    break;
                }
                System::Delay(1);
            }
        }
        return return_value;
    }

    /**  Reads the factory-set coefficients */
    void ReadCoefficients()
    {
        _bme280_calib.dig_T1 = Read16_LE(BME280_REGISTER_DIG_T1);
        _bme280_calib.dig_T2 = ReadS16_LE(BME280_REGISTER_DIG_T2);
        _bme280_calib.dig_T3 = ReadS16_LE(BME280_REGISTER_DIG_T3);

        _bme280_calib.dig_P1 = Read16_LE(BME280_REGISTER_DIG_P1);
        _bme280_calib.dig_P2 = ReadS16_LE(BME280_REGISTER_DIG_P2);
        _bme280_calib.dig_P3 = ReadS16_LE(BME280_REGISTER_DIG_P3);
        _bme280_calib.dig_P4 = ReadS16_LE(BME280_REGISTER_DIG_P4);
        _bme280_calib.dig_P5 = ReadS16_LE(BME280_REGISTER_DIG_P5);
        _bme280_calib.dig_P6 = ReadS16_LE(BME280_REGISTER_DIG_P6);
        _bme280_calib.dig_P7 = ReadS16_LE(BME280_REGISTER_DIG_P7);
        _bme280_calib.dig_P8 = ReadS16_LE(BME280_REGISTER_DIG_P8);
        _bme280_calib.dig_P9 = ReadS16_LE(BME280_REGISTER_DIG_P9);

        _bme280_calib.dig_H1 = Read8(BME280_REGISTER_DIG_H1);
        _bme280_calib.dig_H2 = ReadS16_LE(BME280_REGISTER_DIG_H2);
        _bme280_calib.dig_H3 = Read8(BME280_REGISTER_DIG_H3);
        _bme280_calib.dig_H4 = ((int8_t)Read8(BME280_REGISTER_DIG_H4) << 4)
                               | (Read8(BME280_REGISTER_DIG_H4 + 1) & 0xF);
        _bme280_calib.dig_H5 = ((int8_t)Read8(BME280_REGISTER_DIG_H5 + 1) << 4)
                               | (Read8(BME280_REGISTER_DIG_H5) >> 4);
        _bme280_calib.dig_H6 = (int8_t)Read8(BME280_REGISTER_DIG_H6);
    }

    /** return true if chip is busy reading cal data
        \returns true if reading calibration, false otherwise
    */
    bool isReadingCalibration()
    {
        uint8_t const rStatus = Read8(BME280_REGISTER_STATUS);

        return (rStatus & (1 << 0)) != 0;
    }

    /**  Returns the temperature from the sensor
        \returns the temperature read from the device
    */
    float ReadTemperature()
    {
        int32_t var1, var2;

        int32_t adc_T = Read24(BME280_REGISTER_TEMPDATA);
        if(adc_T == 0x800000) // value in case temp measurement was disabled
            return NAN;
        adc_T >>= 4;

        var1 = (int32_t)((adc_T / 8) - ((int32_t)_bme280_calib.dig_T1 * 2));
        var1 = (var1 * ((int32_t)_bme280_calib.dig_T2)) / 2048;
        var2 = (int32_t)((adc_T / 16) - ((int32_t)_bme280_calib.dig_T1));
        var2 = (((var2 * var2) / 4096) * ((int32_t)_bme280_calib.dig_T3))
               / 16384;

        t_fine = var1 + var2 + t_fine_adjust;

        int32_t T = (t_fine * 5 + 128) / 256;

        return (float)T / 100;
    }

    /**  Returns the pressure from the sensor
        \returns the pressure value (in Pascal) read from the device
    */
    float ReadPressure()
    {
        int64_t var1, var2, var3, var4;

        ReadTemperature(); // must be done first to get t_fine

        int32_t adc_P = Read24(BME280_REGISTER_PRESSUREDATA);
        if(adc_P == 0x800000) // value in case pressure measurement was disabled
            return NAN;
        adc_P >>= 4;

        var1 = ((int64_t)t_fine) - 128000;
        var2 = var1 * var1 * (int64_t)_bme280_calib.dig_P6;
        var2 = var2 + ((var1 * (int64_t)_bme280_calib.dig_P5) * 131072);
        var2 = var2 + (((int64_t)_bme280_calib.dig_P4) * 34359738368);
        var1 = ((var1 * var1 * (int64_t)_bme280_calib.dig_P3) / 256)
               + ((var1 * ((int64_t)_bme280_calib.dig_P2) * 4096));
        var3 = ((int64_t)1) * 140737488355328;
        var1 = (var3 + var1) * ((int64_t)_bme280_calib.dig_P1) / 8589934592;

        if(var1 == 0)
        {
            return 0; // avoid exception caused by division by zero
        }

        var4 = 1048576 - adc_P;
        var4 = (((var4 * 2147483648) - var2) * 3125) / var1;
        var1 = (((int64_t)_bme280_calib.dig_P9) * (var4 / 8192) * (var4 / 8192))
               / 33554432;
        var2 = (((int64_t)_bme280_calib.dig_P8) * var4) / 524288;
        var4 = ((var4 + var1 + var2) / 256)
               + (((int64_t)_bme280_calib.dig_P7) * 16);

        float P = var4 / 256.0;

        return P;
    }

    /** Returns the humidity from the sensor
        \returns the humidity value read from the device
    */
    float ReadHumidity()
    {
        int32_t var1, var2, var3, var4, var5;

        ReadTemperature(); // must be done first to get t_fine

        int32_t adc_H = Read16(BME280_REGISTER_HUMIDDATA);
        if(adc_H == 0x8000) // value in case humidity measurement was disabled
            return NAN;

        var1 = t_fine - ((int32_t)76800);
        var2 = (int32_t)(adc_H * 16384);
        var3 = (int32_t)(((int32_t)_bme280_calib.dig_H4) * 1048576);
        var4 = ((int32_t)_bme280_calib.dig_H5) * var1;
        var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
        var2 = (var1 * ((int32_t)_bme280_calib.dig_H6)) / 1024;
        var3 = (var1 * ((int32_t)_bme280_calib.dig_H3)) / 2048;
        var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
        var2 = ((var4 * ((int32_t)_bme280_calib.dig_H2)) + 8192) / 16384;
        var3 = var5 * var2;
        var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
        var5 = var3 - ((var4 * ((int32_t)_bme280_calib.dig_H1)) / 16);
        var5 = (var5 < 0 ? 0 : var5);
        var5 = (var5 > 419430400 ? 419430400 : var5);
        uint32_t H = (uint32_t)(var5 / 4096);

        return (float)H / 1024.0;
    }

    /** Calculates the altitude (in meters) from the specified atmospheric
        pressure (in hPa), and sea-level pressure (in hPa).
        \param  seaLevel      Sea-level pressure in hPa
        \returns the altitude value read from the device
    */
    float ReadAltitude(float seaLevel)
    {
        // Equation taken from BMP180 datasheet (page 16):
        //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

        // Note that using the equation from wikipedia can give bad results
        // at high altitude. See this thread for more information:
        //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

        float atmospheric = ReadPressure() / 100.0F;
        return 44330.0 * (1.0 - pow(atmospheric / seaLevel, 0.1903));
    }

    /** Calculates the pressure at sea level (in hPa) from the specified
        altitude (in meters), and atmospheric pressure (in hPa).
        \param  altitude      Altitude in meters
        \param  atmospheric   Atmospheric pressure in hPa
        \returns the pressure at sea level (in hPa) from the specified altitude
    */
    float SeaLevelForAltitude(float altitude, float atmospheric)
    {
        // Equation taken from BMP180 datasheet (page 17):
        //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

        // Note that using the equation from wikipedia can give bad results
        // at high altitude. See this thread for more information:
        //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

        return atmospheric / pow(1.0 - (altitude / 44330.0), 5.255);
    }

    /** Returns Sensor ID found by init() for diagnostics
        \returns Sensor ID 0x60 for BME280, 0x56, 0x57, 0x58 BMP280
    */
    uint32_t SensorID() { return _sensorID; }

    /** Returns the current temperature compensation value in degrees Celcius
        \returns the current temperature compensation value in degrees Celcius
    */
    float GetTemperatureCompensation()
    {
        return float((t_fine_adjust * 5) >> 8) / 100.0;
    }

    /** Sets a value to be added to each temperature reading. This adjusted
        temperature is used in pressure and humidity readings.
        \param  adjustment  Value to be added to each tempature reading in Celcius
    */
    void SetTemperatureCompensation(float adjustment)
    {
        // convert the value in C into and adjustment to t_fine
        t_fine_adjust = ((int32_t(adjustment * 100) << 8)) / 5;
    }

    /** Get and reset the transport error flag
        \return Whether the transport has errored since the last check
    */
    Result GetTransportError() { return transport_.GetError() ? ERR : OK; }

  private:
    int32_t _sensorID; //!< ID of the BME Sensor

    // temperature with high resolution, stored as an attribute
    // as this is used for temperature compensation reading
    // humidity and pressure
    int32_t t_fine;

    // add to compensate temp readings and in turn
    // to pressure and humidity readings
    int32_t t_fine_adjust = 0;

    typedef struct
    {
        uint16_t dig_T1; ///< temperature compensation value
        int16_t  dig_T2; ///< temperature compensation value
        int16_t  dig_T3; ///< temperature compensation value

        uint16_t dig_P1; ///< pressure compensation value
        int16_t  dig_P2; ///< pressure compensation value
        int16_t  dig_P3; ///< pressure compensation value
        int16_t  dig_P4; ///< pressure compensation value
        int16_t  dig_P5; ///< pressure compensation value
        int16_t  dig_P6; ///< pressure compensation value
        int16_t  dig_P7; ///< pressure compensation value
        int16_t  dig_P8; ///< pressure compensation value
        int16_t  dig_P9; ///< pressure compensation value

        uint8_t dig_H1; ///< humidity compensation value
        int16_t dig_H2; ///< humidity compensation value
        uint8_t dig_H3; ///< humidity compensation value
        int16_t dig_H4; ///< humidity compensation value
        int16_t dig_H5; ///< humidity compensation value
        int8_t  dig_H6; ///< humidity compensation value
    } bme280_calib_data;
    // here calibration data is stored
    bme280_calib_data _bme280_calib;

    /** \brief  config register */
    struct config
    {
        // inactive duration (standby time) in normal mode
        // 000 = 0.5 ms
        // 001 = 62.5 ms
        // 010 = 125 ms
        // 011 = 250 ms
        // 100 = 500 ms
        // 101 = 1000 ms
        // 110 = 10 ms
        // 111 = 20 ms
        unsigned int
            t_sb : 3; ///< inactive duration (standby time) in normal mode

        // filter settings
        // 000 = filter off
        // 001 = 2x filter
        // 010 = 4x filter
        // 011 = 8x filter
        // 100 and above = 16x filter
        unsigned int filter : 3; ///< filter settings

        // unused - don't set
        unsigned int none : 1;     ///< unused - don't set
        unsigned int spi3w_en : 1; ///< unused - don't set

        /// \return combined config register
        unsigned int get() { return (t_sb << 5) | (filter << 2) | spi3w_en; }
    };
    config _configReg; //!< config register object

    /** \brief  ctrl_meas register */
    struct ctrl_meas
    {
        // temperature oversampling
        // 000 = skipped
        // 001 = x1
        // 010 = x2
        // 011 = x4
        // 100 = x8
        // 101 and above = x16
        unsigned int osrs_t : 3; ///< temperature oversampling

        // pressure oversampling
        // 000 = skipped
        // 001 = x1
        // 010 = x2
        // 011 = x4
        // 100 = x8
        // 101 and above = x16
        unsigned int osrs_p : 3; ///< pressure oversampling

        // device mode
        // 00       = sleep
        // 01 or 10 = forced
        // 11       = normal
        unsigned int mode : 2; ///< device mode

        /// \return combined ctrl register
        unsigned int get() { return (osrs_t << 5) | (osrs_p << 2) | mode; }
    };
    ctrl_meas _measReg; //!< measurement register object


    /** \brief  ctrl_hum register */
    struct ctrl_hum
    {
        /// unused - don't set
        unsigned int none : 5;

        // pressure oversampling
        // 000 = skipped
        // 001 = x1
        // 010 = x2
        // 011 = x4
        // 100 = x8
        // 101 and above = x16
        unsigned int osrs_h : 3; ///< pressure oversampling

        /// \return combined ctrl hum register
        unsigned int get() { return (osrs_h); }
    };
    ctrl_hum _humReg; //!< hum register object

    Config    config_;
    Transport transport_;
};

/** @} */

using Bme280I2C = Bme280<Bme280I2CTransport>;
using Bme280Spi = Bme280<Bme280SpiTransport>;
} // namespace daisy
#endif