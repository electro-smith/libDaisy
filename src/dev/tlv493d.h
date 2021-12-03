#pragma once
#ifndef DSY_TLV493D_H
#define DSY_TLV493D_H

#define TLV493D_ADDRESS1 0x5E
#define TLV493D_ADDRESS2 0x1F
#define TLV493D_BUSIF_READSIZE 10
#define TLV493D_BUSIF_WRITESIZE 4
#define TLV493D_NUM_OF_REGMASKS 25
#define TLV493D_DEFAULTMODE POWERDOWNMODE
#define TLV493D_MEASUREMENT_READOUT 7

#define TLV493D_B_MULT 0.098f
#define TLV493D_TEMP_MULT 1.1
#define TLV493D_TEMP_OFFSET 315

#define REGMASK_READ 0
#define REGMASK_WRITE 1

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** I2C Transport for TLV493D */
class Tlv493dI2CTransport
{
  public:
    Tlv493dI2CTransport() {}
    ~Tlv493dI2CTransport() {}

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
        i2c_.TransmitBlocking(TLV493D_ADDRESS1, data, size, 10);
    }

    void Read(uint8_t *data, uint16_t size)
    {
        i2c_.ReceiveBlocking(TLV493D_ADDRESS1, data, size, 10);
    }

  private:
    I2CHandle i2c_;
};


/** @brief Device support for TLV493D Magnetic Sensor
    @author beserge
    @date December 2021
*/
template <typename Transport>
class Tlv493d
{
  public:
    Tlv493d() {}
    ~Tlv493d() {}

    enum Registers_e
    {
        R_BX1 = 0,
        R_BX2,
        R_BY1,
        R_BY2,
        R_BZ1,
        R_BZ2,
        R_TEMP1,
        R_TEMP2,
        R_FRAMECOUNTER,
        R_CHANNEL,
        R_POWERDOWNFLAG,
        R_RES1,
        R_RES2,
        R_RES3,
        W_PARITY,
        W_ADDR,
        W_INT,
        W_FAST,
        W_LOWPOWER,
        W_TEMP_NEN,
        W_LP_PERIOD,
        W_PARITY_EN,
        W_RES1,
        W_RES2,
        W_RES3
    };

    struct RegMask_t
    {
        uint8_t rw;
        uint8_t byteAdress;
        uint8_t bitMask;
        uint8_t shift;
    };

    typedef struct
    {
        uint8_t  fast;
        uint8_t  lp;
        uint8_t  lpPeriod;
        uint16_t measurementTime;
    } AccessMode_t;

    enum AccessMode_e
    {
        POWERDOWNMODE = 0,
        FASTMODE,
        LOWPOWERMODE,
        ULTRALOWPOWERMODE,
        MASTERCONTROLLEDMODE,
    };

    const AccessMode_t accModes[] = {
        {0, 0, 0, 1000}, // POWERDOWNMODE
        {1, 0, 0, 0},    // FASTMODE
        {0, 1, 1, 10},   // LOWPOWERMODE
        {0, 1, 0, 100},  // ULTRALOWPOWERMODE
        {1, 1, 1, 10}    // MASTERCONTROLLEDMODE
    };

    const RegMask_t RegMasks[] = {
        {REGMASK_READ, 0, 0xFF, 0},  // R_BX1
        {REGMASK_READ, 4, 0xF0, 4},  // R_BX2
        {REGMASK_READ, 1, 0xFF, 0},  // R_BY1
        {REGMASK_READ, 4, 0x0F, 0},  // R_BY2
        {REGMASK_READ, 2, 0xFF, 0},  // R_BZ1
        {REGMASK_READ, 5, 0x0F, 0},  // R_BZ2
        {REGMASK_READ, 3, 0xF0, 4},  // R_TEMP1
        {REGMASK_READ, 6, 0xFF, 0},  // R_TEMP2
        {REGMASK_READ, 3, 0x0C, 2},  // R_FRAMECOUNTER
        {REGMASK_READ, 3, 0x03, 0},  // R_CHANNEL
        {REGMASK_READ, 5, 0x10, 4},  // R_POWERDOWNFLAG
        {REGMASK_READ, 7, 0x18, 3},  // R_RES1
        {REGMASK_READ, 8, 0xFF, 0},  // R_RES2
        {REGMASK_READ, 9, 0x1F, 0},  // R_RES3
        {REGMASK_WRITE, 1, 0x80, 7}, // W_PARITY
        {REGMASK_WRITE, 1, 0x60, 5}, // W_ADDR
        {REGMASK_WRITE, 1, 0x04, 2}, // W_INT
        {REGMASK_WRITE, 1, 0x02, 1}, // W_FAST
        {REGMASK_WRITE, 1, 0x01, 0}, // W_LOWPOWER
        {REGMASK_WRITE, 3, 0x80, 7}, // W_TEMP_EN
        {REGMASK_WRITE, 3, 0x40, 6}, // W_LOWPOWER
        {REGMASK_WRITE, 3, 0x20, 5}, // W_POWERDOWN
        {REGMASK_WRITE, 1, 0x18, 3}, // W_RES1
        {REGMASK_WRITE, 2, 0xFF, 0}, // W_RES2
        {REGMASK_WRITE, 3, 0x1F, 0}  // W_RES3
    };

    struct Config
    {
        typename Transport::Config transport_config;
        uint8_t                    address;
        Config() { address = TLV493D_ADDRESS1; }
    };

    /** Initialize the TLV493D device
        \param config Configuration settings
    */
    void Init(Config config)
    {
        config_ = config;

        transport_.Init(config_.transport_config);

        System::Delay(40); // 40ms startup delay

        ResetSensor(config_.address);

        // get all register data from sensor
        ReadOut();
        // copy factory settings to write registers
        SetRegBits(W_RES1, GetRegBits(R_RES1));
        SetRegBits(W_RES2, GetRegBits(R_RES2));
        SetRegBits(W_RES3, GetRegBits(R_RES3));
        // enable parity detection
        SetRegBits(W_PARITY_EN, 1);
        // config sensor to lowpower mode
        // also contains parity calculation and writeout to sensor
        SetAccessMode(TLV493D_DEFAULTMODE);
    }

    void ReadOut() { transport_.Read(regReadData, TLV493D_BUSIF_READSIZE); }

    void WriteOut() { transport_.Write(regWriteData, TLV493D_BUSIF_WRITESIZE); }

    void SetRegBits(uint8_t regMaskIndex, uint8_t data)
    {
        if(regMaskIndex < TLV493D_NUM_OF_REGMASKS)
        {
            SetToRegs(RegMasks[regMaskIndex], regWriteData, data);
        }
    }

    void GetRegBits(uint8_t regMaskIndex)
    {
        if(regMaskIndex < TLV493D_NUM_OF_REGMASKS)
        {
            const RegMask_t *mask = &(RegMasks[regMaskIndex]);
            if(mask->rw == REGMASK_READ)
            {
                return GetFromRegs(mask, regReadData);
            }
            else
            {
                return GetFromRegs(mask, regWriteData);
            }
        }
    }


    void UpdateData()
    {
        SetAccessMode(MASTERCONTROLLEDMODE);
        System::Delay(GetMeasurementDelay());

        ReadOut(TLV493D_MEASUREMENT_READOUT);

        // construct results from registers
        mXdata = ConcatResults(getRegBits(R_BX1), getRegBits(R_BX2), true);
        mYdata = ConcatResults(getRegBits(R_BY1), getRegBits(R_BY2), true);
        mZdata = ConcatResults(getRegBits(R_BZ1), getRegBits(R_BZ2), true);
        mTempdata
            = ConcatResults(getRegBits(R_TEMP1), getRegBits(R_TEMP2), false);

        SetAccessMode(POWERDOWNMODE);
        GetRegBits(R_CHANNEL);

        mExpectedFrameCount = GetRegBits(R_FRAMECOUNTER) + 1;
    }


    float GetX() { return static_cast<float>(mXdata) * TLV493D_B_MULT; }


    float GetY() { return static_cast<float>(mYdata) * TLV493D_B_MULT; }


    float GetZ() { return static_cast<float>(mZdata) * TLV493D_B_MULT; }


    float GetTemp()
    {
        return static_cast<float>(mTempdata - TLV493D_TEMP_OFFSET)
               * TLV493D_TEMP_MULT;
    }


    float GetAmount()
    {
        // sqrt(x^2 + y^2 + z^2)
        return TLV493D_B_MULT
               * sqrt(pow(static_cast<float>(mXdata), 2)
                      + pow(static_cast<float>(mYdata), 2)
                      + pow(static_cast<float>(mZdata), 2));
    }


    float GetAzimuth()
    {
        // arctan(y/x)
        return atan2(static_cast<float>(mYdata), static_cast<float>(mXdata));
    }


    float GetPolar()
    {
        // arctan(z/(sqrt(x^2+y^2)))
        return atan2(static_cast<float>(mZdata),
                     sqrt(pow(static_cast<float>(mXdata), 2)
                          + pow(static_cast<float>(mYdata), 2)));
    }

    uint16_t GetMeasurementDelay() { return accModes[mMode].measurementTime; }

    void SetAccessMode(AccessMode_e mode)
    {
        const AccessMode_t *modeConfig = &(accModes[mode]);
        SetRegBits(W_FAST, modeConfig->fast);
        SetRegBits(W_LOWPOWER, modeConfig->lp);
        SetRegBits(W_LP_PERIOD, modeConfig->lpPeriod);
        CalcParity();
        WriteOut();
        mMode = mode;
    }

    void CalcParity()
    {
        uint8_t i;
        uint8_t y = 0x00;
        // set parity bit to 1
        // algorithm will calculate an even parity and replace this bit,
        // so parity becomes odd
        SetRegBits(W_PARITY, 1);
        // combine array to one byte first
        for(i = 0; i < TLV493D_BUSIF_WRITESIZE; i++)
        {
            y ^= regWriteData[i];
        }
        // combine all bits of this byte
        y = y ^ (y >> 1);
        y = y ^ (y >> 2);
        y = y ^ (y >> 4);
        // parity is in the LSB of y
        SetRegBits(W_PARITY, y & 0x01);
    }

  private:
    Config    config_;
    Transport transport_;
    uint8_t   regReadData[TLV493D_BUSIF_READSIZE];
    uint8_t   regWriteData[TLV493D_BUSIF_WRITESIZE];
    int16_t   mXdata, mYdata, mZdata, mTempdata, mExpectedFrameCount, mMode;

    // internal function called by begin()
    void ResetSensor(uint8_t adr)
    {
        uint8_t data[2];
        data[0] = 0x00;
        if(adr == TLV493D_ADDRESS1)
        {
            // if the sensor shall be initialized with i2c address 0x1F
            data[1] = 0xFF;
        }
        else
        {
            // if the sensor shall be initialized with address 0x5E
            data[1] = 0x00;
        }

        transport_.Write(data, 2);
    }

    uint8_t GetFromRegs(const RegMask_t *mask, uint8_t *regData)
    {
        return (regData[mask->byteAdress] & mask->bitMask) >> mask->shift;
    }


    void SetToRegs(const RegMask_t *mask, uint8_t *regData, uint8_t toWrite)
    {
        if(mask->rw == REGMASK_WRITE)
        {
            uint8_t regValue = regData[mask->byteAdress];
            regValue &= ~(mask->bitMask);
            regValue |= (toWrite << mask->shift) & mask->bitMask;
            regData[mask->byteAdress] = regValue;
        }
    }
};

/** @} */

using Tlv493dI2C = Tlv493d<Tlv493dI2CTransport>;
} // namespace daisy
#endif
