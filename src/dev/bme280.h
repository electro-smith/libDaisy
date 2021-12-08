#pragma once
#ifndef DSY_Bme280_H
#define DSY_Bme280_H

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
        i2c_.TransmitBlocking(Bme280_CHIP_ID, data, size, 10);
    }

    void Read(uint8_t *data, uint16_t size)
    {
        i2c_.ReceiveBlocking(Bme280_CHIP_ID, data, size, 10);
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
        return buff;
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

  private:
    I2CHandle i2c_;
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
        spi_.BlockingTransmit(data, size);
    }

    void Read(uint8_t *data, uint16_t size)
    {
        spi_.BlockingReceive(data, size, 10);
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
        return buff;
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

  private:
    SpiHandle spi_;
};

/** @brief Device support for BME280 Humidity Pressure Sensor
    @author beserge
    @date December 2021
*/
template <typename Transport>
class Bme280
{
  public:
    Bme280() {}
    ~Bme280() {}

    struct Config
    {
        typename Transport::Config transport_config;

        Config() {}
    };

    /** Initialize the Bme280 device
        \param config Configuration settings
    */
    void Init(Config config)
    {
        config_ = config;

        transport_.Init(config_.transport_config);

        // reset the device using soft-reset
        // this makes sure the IIR is off, etc.
        transport_.Write8((BME280_REGISTER_SOFTRESET, 0xB6);

        // wait for chip to wake up.
        System::Delay(10);

        // if chip is still reading calibration, delay
        while(isReadingCalibration())
            System::Delay(10);

        ReadCoefficients(); // read trimming parameters, see DS 4.2.2

        SetSampling(); // use defaults

        System::Delay(100);
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
        transport_.Write8((BME280_REGISTER_CONTROL, MODE_SLEEP);

        // you must make sure to also set REGISTER_CONTROL after setting the
        // CONTROLHUMID register, otherwise the values won't be applied (see
        // DS 5.4.3)
        transport_.Write8((BME280_REGISTER_CONTROLHUMID, _humReg.get());
        transport_.Write8((BME280_REGISTER_CONFIG, _configReg.get());
        transport_.Write8((BME280_REGISTER_CONTROL, _measReg.get());
    }

    /**  Reads a signed 16 bit little endian value over I2C or SPI
        \param reg the register address to read from
        \returns the 16 bit data value read from the device
    */
    uint16_t Read16_LE(uint8_t reg)
    {
        uint16_t temp = transport_.Read16(reg);
        return (temp >> 8) | (temp << 8);
    }

    /**  Reads a signed 16 bit value over I2C or SPI
        \param reg the register address to read from
        \returns the 16 bit data value read from the device
    */
    int16_t ReadS16(uint8_t reg) { return (int16_t)transport_.Read16(reg); }

    /**  Reads a signed little endian 16 bit value over I2C or SPI
        \param reg the register address to read from
        \returns the 16 bit data value read from the device
    */
    int16_t ReadS16_LE(uint8_t reg) { return (int16_t)Read16_LE(reg); }

    /** Take a new measurement (only possible in forced mode)
        \returns true in case of success else false
    */
    bool TakeForcedMeasurement(void)
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
            transport_.Write8((BME280_REGISTER_CONTROL, _measReg.get());
            // Store current time to measure the timeout
            uint32_t timeout_start = millis();
            // wait until measurement has been completed, otherwise we would read the
            // the values from the last measurement or the timeout occurred after 2 sec.
            while(transport_.Read8((BME280_REGISTER_STATUS) & 0x08)
            {
                // In case of a timeout, stop the while loop
                if((millis() - timeout_start) > 2000)
                {
                    return_value = false;
                    break;
                }
                delay(1);
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

        _bme280_calib.dig_H1 = transport_.Read8((BME280_REGISTER_DIG_H1);
        _bme280_calib.dig_H2 = ReadS16_LE(BME280_REGISTER_DIG_H2);
        _bme280_calib.dig_H3 = transport_.Read8((BME280_REGISTER_DIG_H3);
        _bme280_calib.dig_H4 = ((int8_t)transport_.Read8((BME280_REGISTER_DIG_H4) << 4)
                               | (transport_.Read8((BME280_REGISTER_DIG_H4 + 1) & 0xF);
        _bme280_calib.dig_H5 = ((int8_t)transport_.Read8((BME280_REGISTER_DIG_H5 + 1) << 4)
                               | (transport_.Read8((BME280_REGISTER_DIG_H5) >> 4);
        _bme280_calib.dig_H6 = (int8_t)transport_.Read8((BME280_REGISTER_DIG_H6);
    }

    /** return true if chip is busy reading cal data
        \returns true if reading calibration, false otherwise
    */
    bool isReadingCalibration(void)
    {
        uint8_t const rStatus = transport_.Read8((BME280_REGISTER_STATUS);

        return (rStatus & (1 << 0)) != 0;
    }

    /**  Returns the temperature from the sensor
        \returns the temperature read from the device
    */
    float ReadTemperature(void)
    {
        int32_t var1, var2;

        int32_t adc_T = transport_.Read24(BME280_REGISTER_TEMPDATA);
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
    float ReadPressure(void)
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
    float ReadHumidity(void)
    {
        int32_t var1, var2, var3, var4, var5;

        ReadTemperature(); // must be done first to get t_fine

        int32_t adc_H = transport_.Read16(BME280_REGISTER_HUMIDDATA);
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
    uint32_t SensorID(void) { return _SensorID; }

    /** Returns the current temperature compensation value in degrees Celcius
        \returns the current temperature compensation value in degrees Celcius
    */
    float GetTemperatureCompensation(void)
    {
        return float((t_fine_adjust * 5) >> 8) / 100.0;
    };

    /** Sets a value to be added to each temperature reading. This adjusted
        temperature is used in pressure and humidity readings.
        \param  adjustment  Value to be added to each tempature reading in Celcius
    */
    void SetTemperatureCompensation(float adjustment)
    {
        // convert the value in C into and adjustment to t_fine
        t_fine_adjust = ((int32_t(adjustment * 100) << 8)) / 5;
    };

    /**
    @brief  Gets an Adafruit Unified Sensor object for the temp sensor component
    @return Adafruit_Sensor pointer to temperature sensor
 */
    Adafruit_Sensor *getTemperatureSensor(void)
    {
        if(!temp_sensor)
        {
            temp_sensor = new Adafruit_BME280_Temp(this);
        }

        return temp_sensor;
    }

    /**
    @brief  Gets an Adafruit Unified Sensor object for the pressure sensor
   component
    @return Adafruit_Sensor pointer to pressure sensor
 */
    Adafruit_Sensor *getPressureSensor(void)
    {
        if(!pressure_sensor)
        {
            pressure_sensor = new Adafruit_BME280_Pressure(this);
        }
        return pressure_sensor;
    }

    /**
    @brief  Gets an Adafruit Unified Sensor object for the humidity sensor
   component
    @return Adafruit_Sensor pointer to humidity sensor
 */
    Adafruit_Sensor *getHumiditySensor(void)
    {
        if(!humidity_sensor)
        {
            humidity_sensor = new Adafruit_BME280_Humidity(this);
        }
        return humidity_sensor;
    }

    /**************************************************************************/
    /**
    @brief  Gets the sensor_t data for the BME280's temperature sensor
*/
    /**************************************************************************/
    void Adafruit_BME280_Temp::getSensor(sensor_t *sensor)
    {
        /* Clear the sensor_t object */
        memset(sensor, 0, sizeof(sensor_t));

        /* Insert the sensor name in the fixed length char array */
        strncpy(sensor->name, "BME280", sizeof(sensor->name) - 1);
        sensor->name[sizeof(sensor->name) - 1] = 0;
        sensor->version                        = 1;
        sensor->sensor_id                      = _SensorID;
        sensor->type       = SENSOR_TYPE_AMBIENT_TEMPERATURE;
        sensor->min_delay  = 0;
        sensor->min_value  = -40.0; /* Temperature range -40 ~ +85 C  */
        sensor->max_value  = +85.0;
        sensor->resolution = 0.01; /*  0.01 C */
    }

    /**************************************************************************/
    /**
    @brief  Gets the temperature as a standard sensor event
    \param  event Sensor event object that will be populated
    \returns True
*/
    /**************************************************************************/
    bool Adafruit_BME280_Temp::getEvent(sensors_event_t *event)
    {
        /* Clear the event */
        memset(event, 0, sizeof(sensors_event_t));

        event->version     = sizeof(sensors_event_t);
        event->sensor_id   = _SensorID;
        event->type        = SENSOR_TYPE_AMBIENT_TEMPERATURE;
        event->timestamp   = millis();
        event->temperature = _theBME280->ReadTemperature();
        return true;
    }

    /**************************************************************************/
    /**
    @brief  Gets the sensor_t data for the BME280's pressure sensor
*/
    /**************************************************************************/
    void Adafruit_BME280_Pressure::getSensor(sensor_t *sensor)
    {
        /* Clear the sensor_t object */
        memset(sensor, 0, sizeof(sensor_t));

        /* Insert the sensor name in the fixed length char array */
        strncpy(sensor->name, "BME280", sizeof(sensor->name) - 1);
        sensor->name[sizeof(sensor->name) - 1] = 0;
        sensor->version                        = 1;
        sensor->sensor_id                      = _SensorID;
        sensor->type                           = SENSOR_TYPE_PRESSURE;
        sensor->min_delay                      = 0;
        sensor->min_value                      = 300.0; /* 300 ~ 1100 hPa  */
        sensor->max_value                      = 1100.0;
        sensor->resolution                     = 0.012; /* 0.12 hPa relative */
    }

    /**************************************************************************/
    /**
    @brief  Gets the pressure as a standard sensor event
    \param  event Sensor event object that will be populated
    \returns True
*/
    /**************************************************************************/
    bool Adafruit_BME280_Pressure::getEvent(sensors_event_t *event)
    {
        /* Clear the event */
        memset(event, 0, sizeof(sensors_event_t));

        event->version   = sizeof(sensors_event_t);
        event->sensor_id = _SensorID;
        event->type      = SENSOR_TYPE_PRESSURE;
        event->timestamp = millis();
        event->pressure = _theBME280->ReadPressure() / 100; // convert Pa to hPa
        return true;
    }

    /**************************************************************************/
    /**
    @brief  Gets the sensor_t data for the BME280's humidity sensor
*/
    /**************************************************************************/
    void Adafruit_BME280_Humidity::getSensor(sensor_t *sensor)
    {
        /* Clear the sensor_t object */
        memset(sensor, 0, sizeof(sensor_t));

        /* Insert the sensor name in the fixed length char array */
        strncpy(sensor->name, "BME280", sizeof(sensor->name) - 1);
        sensor->name[sizeof(sensor->name) - 1] = 0;
        sensor->version                        = 1;
        sensor->sensor_id                      = _SensorID;
        sensor->type                           = SENSOR_TYPE_RELATIVE_HUMIDITY;
        sensor->min_delay                      = 0;
        sensor->min_value                      = 0;
        sensor->max_value                      = 100; /* 0 - 100 %  */
        sensor->resolution                     = 3;   /* 3% accuracy */
    }

    /**************************************************************************/
    /**
    @brief  Gets the humidity as a standard sensor event
    \param  event Sensor event object that will be populated
    \returns True
*/
    /**************************************************************************/
    bool Adafruit_BME280_Humidity::getEvent(sensors_event_t *event)
    {
        /* Clear the event */
        memset(event, 0, sizeof(sensors_event_t));

        event->version           = sizeof(sensors_event_t);
        event->sensor_id         = _SensorID;
        event->type              = SENSOR_TYPE_RELATIVE_HUMIDITY;
        event->timestamp         = millis();
        event->relative_humidity = _theBME280->ReadHumidity();
        return true;
    }

  private:
    Config    config_;
    Transport transport_;
};

/** @} */

using Bme280I2C = Bme280<Bme280I2CTransport>;
using Bme280Spi = Bme280<Bme280SpiTransport>;
} // namespace daisy
#endif