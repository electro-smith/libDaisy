#pragma once
#ifndef DSY_MPR121_H
#define DSY_MPR121_H

// The default I2C address
#define MPR121_I2CADDR_DEFAULT 0x5A        ///< default I2C address
#define MPR121_TOUCH_THRESHOLD_DEFAULT 12  ///< default touch threshold value
#define MPR121_RELEASE_THRESHOLD_DEFAULT 6 ///< default relese threshold value

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** I2C Transport for MPR121 */
class Mpr121I2CTransport
{
  public:
    Mpr121I2CTransport() {}
    ~Mpr121I2CTransport() {}

    struct Config
    {
        I2CHandle::Config::Peripheral periph;
        I2CHandle::Config::Speed      speed;
        Pin                           scl;
        Pin                           sda;

        I2CHandle::Config::Mode mode;

        uint8_t dev_addr;

        Config()
        {
            periph = I2CHandle::Config::Peripheral::I2C_1;
            speed  = I2CHandle::Config::Speed::I2C_400KHZ;

            scl = Pin(PORTB, 8);
            sda = Pin(PORTB, 9);

            dev_addr = MPR121_I2CADDR_DEFAULT;
        }
    };

    /** \return Did the transaction error? i.e. Return true if error, false if ok */
    inline bool Init(Config config)
    {
        config_ = config;

        I2CHandle::Config i2c_conf;
        i2c_conf.mode   = I2CHandle::Config::Mode::I2C_MASTER;
        i2c_conf.periph = config.periph;
        i2c_conf.speed  = config.speed;

        i2c_conf.pin_config.scl = config.scl;
        i2c_conf.pin_config.sda = config.sda;

        return I2CHandle::Result::OK != i2c_.Init(i2c_conf);
    }

    /** \return Did the transaction error? i.e. Return true if error, false if ok */
    bool Write(uint8_t *data, uint16_t size)
    {
        return I2CHandle::Result::OK
               != i2c_.TransmitBlocking(config_.dev_addr, data, size, 10);
    }

    /** \return Did the transaction error? i.e. Return true if error, false if ok */
    bool Read(uint8_t *data, uint16_t size)
    {
        return I2CHandle::Result::OK
               != i2c_.ReceiveBlocking(config_.dev_addr, data, size, 10);
    }

  private:
    I2CHandle i2c_;
    Config    config_;
};


/** @brief Device support for MPR121 12x Capacitive Touch Sensor
    @author beserge
    @date December 2021
*/
template <typename Transport>
class Mpr121
{
  public:
    Mpr121() {}
    ~Mpr121() {}

    struct Config
    {
        typename Transport::Config transport_config;
        uint8_t                    touch_threshold;
        uint8_t                    release_threshold;

        Config()
        {
            touch_threshold   = MPR121_TOUCH_THRESHOLD_DEFAULT;
            release_threshold = MPR121_RELEASE_THRESHOLD_DEFAULT;
        }
    };

    enum Result
    {
        OK = 0,
        ERR
    };

    /** Initialize the MPR121 device
        \param config Configuration settings
    */
    Result Init(Config config)
    {
        config_ = config;

        SetTransportErr(transport_.Init(config_.transport_config));

        // soft reset
        WriteRegister(MPR121_SOFTRESET, 0x63);
        System::Delay(1);

        WriteRegister(MPR121_ECR, 0x0);

        // this doesn't work for some reason...
        // uint8_t c = ReadRegister8(MPR121_CONFIG2);

        // if(c != 0x24)
        //     return ERR;

        SetThresholds(config_.touch_threshold, config_.release_threshold);
        WriteRegister(MPR121_MHDR, 0x01);
        WriteRegister(MPR121_NHDR, 0x01);
        WriteRegister(MPR121_NCLR, 0x0E);
        WriteRegister(MPR121_FDLR, 0x00);

        WriteRegister(MPR121_MHDF, 0x01);
        WriteRegister(MPR121_NHDF, 0x05);
        WriteRegister(MPR121_NCLF, 0x01);
        WriteRegister(MPR121_FDLF, 0x00);

        WriteRegister(MPR121_NHDT, 0x00);
        WriteRegister(MPR121_NCLT, 0x00);
        WriteRegister(MPR121_FDLT, 0x00);

        WriteRegister(MPR121_DEBOUNCE, 0);
        WriteRegister(MPR121_CONFIG1, 0x10); // default, 16uA charge current
        WriteRegister(MPR121_CONFIG2, 0x20); // 0.5uS encoding, 1ms period

        // autoconfig
        // WriteRegister(MPR121_AUTOCONFIG0, 0x0B);

        // correct values for Vdd = 3.3V
        // WriteRegister(MPR121_UPLIMIT, 200);     // ((Vdd - 0.7)/Vdd) * 256
        // WriteRegister(MPR121_TARGETLIMIT, 180); // UPLIMIT * 0.9
        // WriteRegister(MPR121_LOWLIMIT, 130);    // UPLIMIT * 0.65
        // autoconfig

        // enable X electrodes and start MPR121
        uint8_t ECR_SETTING
            = 0x80
              + 12; // 5 bits for baseline tracking & proximity disabled + X
                    // amount of electrodes running (12)
        WriteRegister(MPR121_ECR,
                      ECR_SETTING); // start with above ECR setting

        return GetTransportErr();
    }

    /** Set the touch and release thresholds for all 13 channels on the
        device to the passed values.
        \param      touch The touch threshold value from 0 to 255.
        \param      release The release threshold from 0 to 255.
    */
    void SetThresholds(uint8_t touch, uint8_t release)
    {
        // set all thresholds (the same)
        for(uint8_t i = 0; i < 12; i++)
        {
            WriteRegister(MPR121_TOUCHTH_0 + 2 * i, touch);
            WriteRegister(MPR121_RELEASETH_0 + 2 * i, release);
        }
    }


    /** Read the filtered data from channel t. The ADC raw data outputs
        run through 3 levels of digital filtering to filter out the high
        frequency and low frequency noise encountered. For detailed information on
        this filtering see page 6 of the device datasheet.
        \param      t the channel to read
        \returns    the filtered reading as a 10 bit unsigned value
    */
    uint16_t FilteredData(uint8_t t)
    {
        if(t > 12)
            return 0;
        return ReadRegister16(MPR121_FILTDATA_0L + t * 2);
    }

    /** Read the baseline value for the channel. The 3rd level filtered
        result is internally 10bit but only high 8 bits are readable
        from registers 0x1E~0x2A as the baseline value output for each channel.
        \param      t the channel to read.
        \returns    the baseline data that was read
    */
    uint16_t BaselineData(uint8_t t)
    {
        if(t > 12)
            return 0;
        uint16_t bl = ReadRegister8(MPR121_BASELINE_0 + t);
        return (bl << 2);
    }

    /** Read the touch status of all 13 channels as bit values in a 12 bit integer.
        \returns    a 12 bit integer with each bit corresponding to the touch status
        of a sensor. For example, if bit 0 is set then channel 0 of the
        device is currently deemed to be touched.
    */
    uint16_t Touched()
    {
        uint16_t t = ReadRegister16(MPR121_TOUCHSTATUS_L);
        return t & 0x0FFF;
    }

    /** Read the contents of an 8 bit device register.
        \param      reg the register address to read from
        \returns    the 8 bit value that was read.
    */
    uint8_t ReadRegister8(uint8_t reg)
    {
        uint8_t buff;
        SetTransportErr(transport_.Write(&reg, 1));
        SetTransportErr(transport_.Read(&buff, 1));

        return buff;
    }

    /** Read the contents of a 16 bit device register.
        \param      reg the register address to read from
        \returns    the 16 bit value that was read.
    */
    uint16_t ReadRegister16(uint8_t reg)
    {
        uint16_t buff;
        SetTransportErr(transport_.Write(&reg, 1));
        SetTransportErr(transport_.Read((uint8_t *)&buff, 2));

        return buff;
    }

    /** Writes 8-bits to the specified destination register
        \param  reg the register address to write to
        \param  value the value to write
    */
    void WriteRegister(uint8_t reg, uint8_t value)
    {
        // MPR121 must be put in Stop Mode to write to most registers
        bool stop_required = true;

        // first get the current set value of the MPR121_ECR register
        uint8_t ecr_reg = MPR121_ECR;
        uint8_t buff[2] = {ecr_reg, 0x00};

        SetTransportErr(transport_.Write(buff, 1));

        uint8_t ecr_backup;
        SetTransportErr(transport_.Read(&ecr_backup, 1));
        if((reg == MPR121_ECR) || ((0x73 <= reg) && (reg <= 0x7A)))
        {
            stop_required = false;
        }

        if(stop_required)
        {
            // clear this register to set stop mode
            SetTransportErr(transport_.Write(buff, 2));
        }

        buff[0] = reg;
        buff[1] = value;
        SetTransportErr(transport_.Write(buff, 2));

        if(stop_required)
        {
            // write back the previous set ECR settings
            buff[0] = ecr_reg;
            buff[1] = ecr_backup;
            SetTransportErr(transport_.Write(buff, 2));
        }
    }

    /** Device register map */
    enum RegMap
    {
        MPR121_TOUCHSTATUS_L = 0x00,
        MPR121_TOUCHSTATUS_H = 0x01,
        MPR121_FILTDATA_0L   = 0x04,
        MPR121_FILTDATA_0H   = 0x05,
        MPR121_BASELINE_0    = 0x1E,
        MPR121_MHDR          = 0x2B,
        MPR121_NHDR          = 0x2C,
        MPR121_NCLR          = 0x2D,
        MPR121_FDLR          = 0x2E,
        MPR121_MHDF          = 0x2F,
        MPR121_NHDF          = 0x30,
        MPR121_NCLF          = 0x31,
        MPR121_FDLF          = 0x32,
        MPR121_NHDT          = 0x33,
        MPR121_NCLT          = 0x34,
        MPR121_FDLT          = 0x35,

        MPR121_TOUCHTH_0    = 0x41,
        MPR121_RELEASETH_0  = 0x42,
        MPR121_DEBOUNCE     = 0x5B,
        MPR121_CONFIG1      = 0x5C,
        MPR121_CONFIG2      = 0x5D,
        MPR121_CHARGECURR_0 = 0x5F,
        MPR121_CHARGETIME_1 = 0x6C,
        MPR121_ECR          = 0x5E,
        MPR121_AUTOCONFIG0  = 0x7B,
        MPR121_AUTOCONFIG1  = 0x7C,
        MPR121_UPLIMIT      = 0x7D,
        MPR121_LOWLIMIT     = 0x7E,
        MPR121_TARGETLIMIT  = 0x7F,

        MPR121_GPIODIR    = 0x76,
        MPR121_GPIOEN     = 0x77,
        MPR121_GPIOSET    = 0x78,
        MPR121_GPIOCLR    = 0x79,
        MPR121_GPIOTOGGLE = 0x7A,

        MPR121_SOFTRESET = 0x80,
    };

  private:
    Config    config_;
    Transport transport_;
    bool      transport_error_;

    /** Set the global transport_error_ bool */
    void SetTransportErr(bool err) { transport_error_ |= err; }

    /** Get the global transport_error_ bool (as a Result), then reset it */
    Result GetTransportErr()
    {
        Result ret       = transport_error_ ? ERR : OK;
        transport_error_ = false;
        return ret;
    }

}; // class

using Mpr121I2C = Mpr121<Mpr121I2CTransport>;

/** @} */

} // namespace daisy
#endif
