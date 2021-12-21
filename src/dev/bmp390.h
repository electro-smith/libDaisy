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
#define BMP3_LEN_CALIB_DATA UINT8_C(21)

#define BMP3_LEN_P_T_DATA UINT8_C(6)

/**\name Macro to combine two 8 bit data's to form a 16 bit data */
#define BMP3_CONCAT_BYTES(msb, lsb) (((uint16_t)msb << 8) | (uint16_t)lsb)

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
        Pin                           scl;
        Pin                           sda;

        Config()
        {
            periph = I2CHandle::Config::Peripheral::I2C_1;
            speed  = I2CHandle::Config::Speed::I2C_400KHZ;

            scl = Pin(PORTB, 8);
            sda = Pin(PORTB, 9);
        }
    };

    /** \return Did the transaction error? i.e. Return true if error, false if ok */
    inline bool Init(Config config)
    {
        I2CHandle::Config i2c_config;
        i2c_config.mode   = I2CHandle::Config::Mode::I2C_MASTER;
        i2c_config.periph = config.periph;
        i2c_config.speed  = config.speed;

        i2c_config.pin_config.scl = config.scl;
        i2c_config.pin_config.sda = config.sda;

        return I2CHandle::Result::OK != i2c_.Init(i2c_config);
    }

    /** \return Did the transaction error? i.e. Return true if error, false if ok */
    bool Write(uint8_t* data, uint16_t size)
    {
        return I2CHandle::Result::OK
               != i2c_.TransmitBlocking(BMP390_CHIP_ID, data, size, 10);
    }

    /** \return Did the transaction error? i.e. Return true if error, false if ok */
    bool Read(uint8_t* data, uint16_t size)
    {
        return I2CHandle::Result::OK
               != i2c_.ReceiveBlocking(BMP390_CHIP_ID, data, size, 10);
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

    /** \return Did the transaction error? i.e. Return true if error, false if ok */
    inline bool Init(Config config)
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

        return SpiHandle::Result::OK != spi_.Init(spi_conf);
    }

    /** \return Did the transaction error? i.e. Return true if error, false if ok */
    bool Write(uint8_t* data, uint16_t size)
    {
        return SpiHandle::Result::OK != spi_.BlockingTransmit(data, size);
    }

    /** \return Did the transaction error? i.e. Return true if error, false if ok */
    bool Read(uint8_t* data, uint16_t size)
    {
        return SpiHandle::Result::OK != spi_.BlockingReceive(data, size, 10);
    }

  private:
    SpiHandle spi_;
};

/** @brief Device support for BMP390 temperature / pressure sensor.
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

        uint8_t temp_oversampling;
        uint8_t pressure_oversampling;
        uint8_t output_data_rate;
        uint8_t iir_filter_coeff;

        Config()
        {
            temp_oversampling     = 0; // nada
            pressure_oversampling = 0; // zilch
            output_data_rate      = 3; // 25Hz
            iir_filter_coeff      = 0; // zip
        }
    };

    enum Result
    {
        OK = 0,
        ERR
    };

    /** Initialize the BMP390 device
        \param config Configuration settings
    */
    Result Init(Config config)
    {
        config_          = config;
        transport_error_ = false;

        SetTransportErr(transport_.Init(config_.transport_config));

        SoftReset();
        SetOversampling(config_.temp_oversampling,
                        config_.pressure_oversampling);
        SetOutputDataRate(config_.output_data_rate);
        SetIIRFilterCoeff(config_.iir_filter_coeff);

        GetCalibData();

        return GetTransportErr();
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

        SetTransportErr(transport_.Write(write_buff, 3));
    }

    /** Sequential read from register(s).
        \param reg Register to read from
        \param read_buff Buffer to read into
        \param size Number of bytes to read.
    */
    void ReadFromReg(uint8_t reg, uint8_t* read_buff, uint16_t size)
    {
        SetTransportErr(transport_.Write(&reg, 1));
        SetTransportErr(transport_.Read(read_buff, size));
    }

    /** Performs a pressure reading
        \return Pressure in Pascals
    */
    uint32_t ReadPressure() { return pressure_; }

    /** Performs a temperature reading
        \return Temperature in degrees Centigrade
    */
    uint32_t ReadTemperature() { return temperature_; }

    /** Calculates the altitude in meters 
        \param sealevel Sea-level pressure in hPa
        \return altitude in meters
    */
    float ReadAltitude(float sealevel)
    {
        float atmospheric = ReadPressure() / 100.f;
        return 44330.f * (1.f - pow(atmospheric / sealevel, 0.1903f));
    }

    /** Read and compensate the data for the getters */
    void Process()
    {
        uint8_t reg_data[BMP3_LEN_P_T_DATA];
        ReadFromReg(BMP3_REG_DATA, reg_data, BMP3_LEN_P_T_DATA);
        ParseData(reg_data);
        CompensateData();
    }

  private:
    bool transport_error_;


    /** Quantized Trim Variables */
    struct bmp3_calib_data
    {
        /** @ Quantized Trim Variables */

        /**@{*/
        float par_t1;
        float par_t2;
        float par_t3;
        float par_p1;
        float par_p2;
        float par_p3;
        float par_p4;
        float par_p5;
        float par_p6;
        float par_p7;
        float par_p8;
        float par_p9;
        float par_p10;
        float par_p11;
        float t_lin;

        /**@}*/
    };

    bmp3_calib_data calib_data_;

    /** Set the global transport_error_ bool */
    void SetTransportErr(bool err) { transport_error_ |= err; }

    /** Get the global transport_error_ bool (as a Result), then reset it */
    Result GetTransportErr()
    {
        Result ret       = transport_error_ ? ERR : OK;
        transport_error_ = false;
        return ret;
    }

    /** Stuff the raw bytes into usable temp and pressure vars */
    void ParseData(uint8_t* reg_data)
    {
        uint32_t data_xlsb;
        uint32_t data_lsb;
        uint32_t data_msb;

        /* Store the parsed register values for pressure data */
        data_xlsb     = (uint32_t)reg_data[0];
        data_lsb      = (uint32_t)reg_data[1] << 8;
        data_msb      = (uint32_t)reg_data[2] << 16;
        raw_pressure_ = data_msb | data_lsb | data_xlsb;

        /* Store the parsed register values for temperature data */
        data_xlsb        = (uint32_t)reg_data[3];
        data_lsb         = (uint32_t)reg_data[4] << 8;
        data_msb         = (uint32_t)reg_data[5] << 16;
        raw_temperature_ = data_msb | data_lsb | data_xlsb;
    }

    /** Compensate the data based on calibration data read in from IC.
    Haven't set the calib data read up yet, so for now this does nothing */
    void CompensateData()
    {
        /* If pressure or temperature component is selected */
        if(config_.compensate_pressure | config_.compensate_temp)
        {
            /* Compensate the temperature data */
            CompensateTemperature();
        }

        if(config_.compensate_pressure)
        {
            /* Compensate the pressure data */
            CompensatePressure();
        }
    }

    /** This internal API is used to compensate the raw temperature data and
        return the compensated temperature data in float data type.
        for e.g. returns temperature 24.26 deg Celsius
    */
    void CompensateTemperature()
    {
        uint32_t uncomp_temp = raw_temperature_;
        float    partial_data1;
        float    partial_data2;

        partial_data1 = (float)(uncomp_temp - calib_data_.par_t1);
        partial_data2 = (float)(partial_data1 * calib_data_.par_t2);

        /* Update the compensated temperature in calib structure since this is
            needed for pressure calculation */
        calib_data_.t_lin
            = partial_data2
              + (partial_data1 * partial_data1) * calib_data_.par_t3;

        /* Returns compensated temperature */
        temperature_ = calib_data_.t_lin;
    }

    /** This internal API is used to compensate the raw pressure data and
        return the compensated pressure data in float data type.
        For e.g. returns pressure in Pascal p = 95305.295 which is 953.05295 hecto pascal
    */
    void CompensatePressure()
    {
        uint32_t uncomp_press = raw_pressure_;

        /* Temporary variables used for compensation */
        float partial_data1;
        float partial_data2;
        float partial_data3;
        float partial_data4;
        float partial_out1;
        float partial_out2;

        partial_data1 = calib_data_.par_p6 * calib_data_.t_lin;
        partial_data2 = calib_data_.par_p7 * pow_bmp3(calib_data_.t_lin, 2);
        partial_data3 = calib_data_.par_p8 * pow_bmp3(calib_data_.t_lin, 3);
        partial_out1  = calib_data_.par_p5 + partial_data1 + partial_data2
                       + partial_data3;
        partial_data1 = calib_data_.par_p2 * calib_data_.t_lin;
        partial_data2 = calib_data_.par_p3 * pow_bmp3(calib_data_.t_lin, 2);
        partial_data3 = calib_data_.par_p4 * pow_bmp3(calib_data_.t_lin, 3);
        partial_out2  = uncomp_press
                       * (calib_data_.par_p1 + partial_data1 + partial_data2
                          + partial_data3);
        partial_data1 = pow_bmp3((float)uncomp_press, 2);
        partial_data2
            = calib_data_.par_p9 + calib_data_.par_p10 * calib_data_.t_lin;
        partial_data3 = partial_data1 * partial_data2;
        partial_data4
            = partial_data3
              + pow_bmp3((float)uncomp_press, 3) * calib_data_.par_p11;
        pressure_ = partial_out1 + partial_out2 + partial_data4;
    }

    /** This internal API is used to calculate the power functionality for
        floating point values.
    */
    float pow_bmp3(float base, uint8_t power)
    {
        float pow_output = 1;

        while(power != 0)
        {
            pow_output = (float)base * pow_output;
            power--;
        }

        return pow_output;
    }


    /** This internal API is used to parse the calibration data, compensates it and store it in device structure */
    void ParseCalibData(const uint8_t* reg_data)
    {
        /* Temporary variable */
        float temp_var;

        /* 1 / 2^8 */
        temp_var = 0.00390625f;
        calib_data_.par_t1
            = (float)BMP3_CONCAT_BYTES(reg_data[1], reg_data[0]) / temp_var;

        temp_var = 1073741824.0f;
        calib_data_.par_t2
            = (float)BMP3_CONCAT_BYTES(reg_data[3], reg_data[2]) / temp_var;

        temp_var           = 281474976710656.0f;
        calib_data_.par_t3 = (float)reg_data[4] / temp_var;

        temp_var           = 1048576.0f;
        calib_data_.par_p1 = (float)BMP3_CONCAT_BYTES(reg_data[6], reg_data[5]);
        calib_data_.par_p1 = (calib_data_.par_p1 - (16384.f)) / temp_var;

        temp_var           = 536870912.0f;
        calib_data_.par_p2 = (float)BMP3_CONCAT_BYTES(reg_data[8], reg_data[7]);
        calib_data_.par_p2 = (calib_data_.par_p2 - (16384.f)) / temp_var;

        temp_var           = 4294967296.0f;
        calib_data_.par_p3 = (float)reg_data[9] / temp_var;

        temp_var           = 137438953472.0f;
        calib_data_.par_p4 = (float)reg_data[10] / temp_var;

        /* 1 / 2^3 */
        temp_var = 0.125f;
        calib_data_.par_p5
            = (float)BMP3_CONCAT_BYTES(reg_data[12], reg_data[11]) / temp_var;

        temp_var = 64.0f;
        calib_data_.par_p6
            = (float)BMP3_CONCAT_BYTES(reg_data[14], reg_data[13]) / temp_var;

        temp_var           = 256.0f;
        calib_data_.par_p7 = (float)reg_data[15] / temp_var;

        temp_var           = 32768.0f;
        calib_data_.par_p8 = (float)reg_data[16] / temp_var;

        temp_var = 281474976710656.0f;
        calib_data_.par_p9
            = (float)BMP3_CONCAT_BYTES(reg_data[18], reg_data[17]) / temp_var;

        temp_var            = 281474976710656.0f;
        calib_data_.par_p10 = (float)reg_data[19] / temp_var;

        temp_var            = 36893488147419103232.0f;
        calib_data_.par_p11 = (float)reg_data[20] / temp_var;
    }

    /** This internal API reads the calibration data from the sensor, parse
        it then compensates it and store in the device structure.
    */
    void GetCalibData()
    {
        uint8_t reg_addr = BMP3_REG_CALIB_DATA;

        /* Array to store calibration data */
        uint8_t calib_data[BMP3_LEN_CALIB_DATA] = {0};

        /* Read the calibration data from the sensor */
        ReadFromReg(reg_addr, calib_data, BMP3_LEN_CALIB_DATA);

        /* Parse calibration data and store it in device structure */
        ParseCalibData(calib_data);
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
    uint32_t  raw_temperature_, raw_pressure_;
};

/** @} */

using Bmp390I2C = Bmp390<Bmp390I2CTransport>;
using Bmp390Spi = Bmp390<Bmp390SpiTransport>;
} // namespace daisy
#endif
