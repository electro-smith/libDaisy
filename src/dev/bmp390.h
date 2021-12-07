#pragma once
#ifndef DSY_BMP390_H
#define DSY_BMP390_H

#define BMP3_IIR_FILTER_DISABLE UINT8_C(0x00)
#define BMP3_ODR_25_HZ UINT8_C(0x03)
#define BMP390_CHIP_ID UINT8_C(0x60)
#define BMP390_SLAVE_ADD UINT8_C(0xEC)

#define BMP3_REG_DATA UINT8_C(0x04)
#define BMP3_REG_OSR UINT8_C(0x1C)
#define BMP3_REG_ODR UINT8_C(0x1D)
#define BMP3_REG_IIR UINT8_C(0x1F)
#define BMP3_REG_CONFIG UINT8_C(0x1F)
#define BMP3_REG_CALIB_DATA UINT8_C(0x31)
#define BMP3_REG_CMD UINT8_C(0x7E)

#define BMP3_LEN_P_T_DATA UINT8_C(6)

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** I2C Transport for BMP390 */
class Bmp390I2CTransport
{
  public:
    Bmp390I2CTransport() {}
    ~Bmp390I2CTransport() {}

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
        i2c_.TransmitBlocking(BMP390_CHIP_ID, data, size, 10);
    }

    void Read(uint8_t* data, uint16_t size)
    {
        i2c_.ReceiveBlocking(BMP390_CHIP_ID, data, size, 10);
    }

  private:
    I2CHandle i2c_;
};

/** SPI Transport for BMP390 */
class Bmp390SpiTransport
{
  public:
    Bmp390SpiTransport() {}
    ~Bmp390SpiTransport() {}

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

    void Write(uint8_t* data, uint16_t size)
    {
        spi_.BlockingTransmit(data, size);
    }

    void Read(uint8_t* data, uint16_t size)
    {
        spi_.BlockingReceive(data, size, 10);
    }

  private:
    SpiHandle spi_;
};

/** @brief Device support for CMP390 temperature / pressure sensor.
    Class is based on Adafruit arduino support, and Bosch datasheet.
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
        bool                       compensate_pressure;
        bool                       compensate_temp;
    };

    /** Initialize the BMP390 device
        \param config Configuration settings
    */
    void Init(Config config)
    {
        config_ = config;

        transport_.Init(config_.transport_config);

        SoftReset();
        SetOversampling(0, 0); // no oversampling for temp or pressure
        SetOutputDataRate(3);  // 25 Hz
        SetIIRFilterCoeff(0);  // disabled
    }

    /** Set the output data rate. 
        Subdivision factor for pressure and temperature measurements is
        2^value. Allowed values are 0...17. Other values are saturated at 17. 
    */
    void SetOutputDataRate(uint8_t rate)
    {
        if(rate > 17)
            rate = 17;

        WriteToReg(BMP3_REG_ODR, rate);
    }

    /** Set the IIR Filter Coefficient
        The inputs map to coeff. as follows:
        (0,7) -> {bypass, 1, 3, 7, 15, 31, 63, 127} 
        \param coeff IIR Filter Coefficient (follows mapping above)
    */
    void SetIIRFilterCoeff(uint8_t coeff)
    {
        if(coeff > 7)
            coeff = 7;

        WriteToReg(BMP3_REG_IIR, coeff);
    }

    /** Set the temperature and pressure oversampling
        The values map in the follwing way: 
        (0,5) -> {1x, 2x, 4x, 8x, 16x, 32x} 
        \param temp Temperature oversampling (follows mapping above)
        \param press Pressure oversampling (follows mapping above)
    */
    void SetOversampling(uint8_t temp, uint8_t press)
    {
        if(temp > 5)
            temp = 5;

        if(press > 5)
            press = 5;

        uint8_t val = temp | press << 3;

        WriteToReg(BMP3_REG_OSR, val);
    }

    /** Write to a single register
        \param reg Register to write to
        \param data Data to write
    */
    void WriteToReg(uint8_t reg, uint8_t data)
    {
        uint8_t write_buff[3];

        write_buff[0] = BMP390_SLAVE_ADD
                        | ((uint8_t)sdo_state_
                           << 1); // slave address | SDO state (111011X0)
        write_buff[1] = reg;      // register address
        write_buff[2] = data;     // value to write

        transport_.Write(write_buff, 3);
    }

    /** Sequential read from register(s).
        \param reg Register to read from
        \param read_buff Buffer to read into
        \param size Number of bytes to read.
    */
    void ReadFromReg(uint8_t reg, uint8_t* read_buff, uint16_t size)
    {
        transport_.Write(&reg, 1);
        transport_.Read(read_buff, size);
    }

    /** Performs a pressure reading
        \return Pressure in Pascals
    */
    uint32_t ReadPressure()
    {
        PerformReading();
        return pressure_;
    }

    /** Performs a temperature reading
        \return Temperature in degrees Centigrade
    */
    uint32_t ReadTemperature()
    {
        PerformReading();
        return temperature_;
    }

    /** Calculates the altitude in meters 
        \param sealevel Sea-level pressure in hPa
        \return altitude in meters
    */
    float ReadAltitude(float sealevel)
    {
        float atmospheric = ReadPressure() / 100.f;
        return 44330.f * (1.f - pow(atmospheric / sealevel, 0.1903f));
    }

  private:
    /** Do all the steps involved in a read */
    void PerformReading()
    {
        uint8_t reg_data[BMP3_LEN_P_T_DATA];
        ReadFromReg(BMP3_REG_DATA, reg_data, BMP3_LEN_P_T_DATA);
        ParseData(reg_data);
        CompensateData();
    }

    /** Stuff the raw bytes into usable temp and pressure vars */
    void ParseData(uint8_t* reg_data)
    {
        uint32_t data_xlsb;
        uint32_t data_lsb;
        uint32_t data_msb;

        /* Store the parsed register values for pressure data */
        data_xlsb = (uint32_t)reg_data[0];
        data_lsb  = (uint32_t)reg_data[1] << 8;
        data_msb  = (uint32_t)reg_data[2] << 16;
        pressure_ = data_msb | data_lsb | data_xlsb;

        /* Store the parsed register values for temperature data */
        data_xlsb    = (uint32_t)reg_data[3];
        data_lsb     = (uint32_t)reg_data[4] << 8;
        data_msb     = (uint32_t)reg_data[5] << 16;
        temperature_ = data_msb | data_lsb | data_xlsb;
    }

    /** Compensate the data based on calibration data read in from IC.
    Haven't set the calib data read up yet, so for now this does nothing */
    void CompensateData()
    {
        /* If pressure or temperature component is selected */
        if(config_.compensate_pressure | config_.compensate_temp)
        {
            /* Compensate the temperature data */
            // temperature = compensate_temperature(uncomp_data, calib_data);
        }

        if(config_.compensate_pressure)
        {
            /* Compensate the pressure data */
            // pressure_ = compensate_pressure(uncomp_data, calib_data);
        }
    }

    void SoftReset()
    {
        // soft reset command is 0xB6
        WriteToReg(BMP3_REG_CMD, 0xB6);
    }

    bool      sdo_state_;
    Config    config_;
    Transport transport_;
    float     temperature_, pressure_;
};

/** @} */

using Bmp390I2C = Bmp390<Bmp390I2CTransport>;
using Bmp390Spi = Bmp390<Bmp390SpiTransport>;
} // namespace daisy
#endif
