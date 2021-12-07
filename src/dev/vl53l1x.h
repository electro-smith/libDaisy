#pragma once
#ifndef DSY_H
#define DSY_H

#define I2C_SLAVE__DEVICE_ADDRESS 0x0001
#define VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND 0x0008
#define ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS 0x0016
#define ALGO__CROSSTALK_COMPENSATION_X_PLANE_GRADIENT_KCPS 0x0018
#define ALGO__CROSSTALK_COMPENSATION_Y_PLANE_GRADIENT_KCPS 0x001A
#define ALGO__PART_TO_PART_RANGE_OFFSET_MM 0x001E
#define MM_CONFIG__INNER_OFFSET_MM 0x0020
#define MM_CONFIG__OUTER_OFFSET_MM 0x0022
#define GPIO_HV_MUX__CTRL 0x0030
#define GPIO__TIO_HV_STATUS 0x0031
#define SYSTEM__INTERRUPT_CONFIG_GPIO 0x0046
#define PHASECAL_CONFIG__TIMEOUT_MACROP 0x004B
#define RANGE_CONFIG__TIMEOUT_MACROP_A_HI 0x005E
#define RANGE_CONFIG__VCSEL_PERIOD_A 0x0060
#define RANGE_CONFIG__VCSEL_PERIOD_B 0x0063
#define RANGE_CONFIG__TIMEOUT_MACROP_B_HI 0x0061
#define RANGE_CONFIG__TIMEOUT_MACROP_B_LO 0x0062
#define RANGE_CONFIG__SIGMA_THRESH 0x0064
#define RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS 0x0066
#define RANGE_CONFIG__VALID_PHASE_HIGH 0x0069
#define SYSTEM__INTERMEASUREMENT_PERIOD 0x006C
#define SYSTEM__THRESH_HIGH 0x0072
#define SYSTEM__THRESH_LOW 0x0074
#define SD_CONFIG__WOI_SD0 0x0078
#define SD_CONFIG__INITIAL_PHASE_SD0 0x007A
#define ROI_CONFIG__USER_ROI_CENTRE_SPAD 0x007F
#define ROI_CONFIG__USER_ROI_REQUESTED_GLOBAL_XY_SIZE 0x0080
#define SYSTEM__SEQUENCE_CONFIG 0x0081
#define SYSTEM__GROUPED_PARAMETER_HOLD 0x0082
#define SYSTEM__INTERRUPT_CLEAR 0x0086
#define SYSTEM__MODE_START 0x0087
#define RESULT__RANGE_STATUS 0x0089
#define RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0 0x008C
#define RESULT__AMBIENT_COUNT_RATE_MCPS_SD 0x0090
#define RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0 0x0096
#define RESULT__PEAK_SIGNAL_COUNT_RATE_CROSSTALK_CORRECTED_MCPS_SD0 0x0098
#define RESULT__OSC_CALIBRATE_VAL 0x00DE
#define FIRMWARE__SYSTEM_STATUS 0x00E5
#define IDENTIFICATION__MODEL_ID 0x010F
#define ROI_CONFIG__MODE_ROI_CENTRE_SPAD 0x013E


#define DEFAULT_DEVICE_ADDRESS 0x52


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

            dev_addr = DEFAULT_DEVICE_ADDRESS;
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
            tmp = CheckForDataReady();
        }

        ClearInterrupt();
        StopRanging();

        // two bounds VHV
        Write8(VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND, 0x09);

        // start VHV from the previous temperature
        Write8(0x0B, 0);
    }

    uint16_t GetDistance()
    {
        return Read8(RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0);
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

        IntPol = GetInterruptPolarity();
        Temp   = Read8(GPIO__TIO_HV_STATUS);

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
        uint8_t buff[] = {index >> 8, index & 0xff};
        uint8_t ret;

        transport_.Write(buff, 2);
        transport_.Read(ret, 1);

        return ret;
    }

    void Write8(uint8_t devaddr, uint16_t index, uint8_t data)
    {
        uint8_t buff[] = {index >> 8, index & 0xFF, data};
        transport_.Write(buff, 3);
    }

    uint16_t ReadWord(uint16_t index)
    {
        uint8_t  buff[] = {index >> 8, index & 0xff};
        uint16_t ret;

        transport_.Write(buff, 2);
        transport_.Read(&ret, 2); // is this ok?

        return ret;
    }

    void WriteWord(uint16_t index, uint16_t data)
    {
        uint8_t buff[] = {index >> 8, index & 0xFF, data >> 8, data & 0xFF};
        transport_.Write(buff, 4);
    }

    void WriteDWord(uint16_t index, uint32_t data)
    {
        int     status;
        uint8_t buff[6];

        buff[0] = index >> 8;
        buff[1] = index & 0xff;
        buff[2] = (data >> 24) & 0xFF;
        buff[3] = (data >> 16) & 0xFF;
        buff[4] = (data >> 8) & 0xFF;
        buff[5] = (data >> 0) & 0xFF;

        transport_.Write(buff, 6);
    }

    uint32_t ReadDWord(uint16_t index)
    {
        uint8_t  buff[] = {index >> 8, index & 0xff};
        uint32_t ret;

        transport_.Write(buff, 2);
        transport_.Read(&ret, 4); // how about this?

        return ret;
    }

    void ClearInterrupt() { Write8(SYSTEM__INTERRUPT_CLEAR, 0x01); }

    void SetTimingBudgetInMs(uint16_t TimingBudgetInMs)
    {
        uint16_t DM;

        DM = GetDistanceMode();
        if(DM == 0)
            return;
        else if(DM == 1) /* Short DistanceMode */
        {
            switch(TimingBudgetInMs)
            {
                case 15: /* only available in short distance mode */
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x01D);
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x0027);
                    break;
                case 20:
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x0051);
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x006E);
                    break;
                case 33:
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x00D6);
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x006E);
                    break;
                case 50:
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x1AE);
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x01E8);
                    break;
                case 100:
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x02E1);
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x0388);
                    break;
                case 200:
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x03E1);
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x0496);
                    break;
                case 500:
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x0591);
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x05C1);
                    break;
                default: 1; break;
            }
        }
        else
        {
            switch(TimingBudgetInMs)
            {
                case 20:
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x001E);
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x0022);
                    break;
                case 33:
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x0060);
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x006E);
                    break;
                case 50:
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x00AD);
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x00C6);
                    break;
                case 100:
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x01CC);
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x01EA);
                    break;
                case 200:
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x02D9);
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x02F8);
                    break;
                case 500:
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x048F);
                    WriteWord(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x04A4);
                    break;
                default: 1; break;
            }
        }
    }

    uint16_t GetTimingBudgetInMs(uint16_t *pTimingBudget)
    {
        uint16_t Temp;

        Temp = Read8(RANGE_CONFIG__TIMEOUT_MACROP_A_HI);
        switch(Temp)
        {
            case 0x001D: return 15;
            case 0x0051: // fall through
            case 0x001E: return 20;
            case 0x00D6: // fall through
            case 0x0060: return 33;
            case 0x1AE: // fall through
            case 0x00AD: return 50;
            case 0x02E1: // fall through
            case 0x01CC: return 100;
            case 0x03E1: // fall through
            case 0x02D9: return 200;
            case 0x0591: // fall through
            case 0x048F: return 500;
            default: return 0;
        }
        return 0;
    }


    void SetDistanceMode(uint16_t DM)
    {
        uint16_t TB;

        TB = GetTimingBudgetInMs();

        switch(DM)
        {
            case 1:
                Write8(PHASECAL_CONFIG__TIMEOUT_MACROP, 0x14);
                Write8(RANGE_CONFIG__VCSEL_PERIOD_A, 0x07);
                Write8(RANGE_CONFIG__VCSEL_PERIOD_B, 0x05);
                Write8(RANGE_CONFIG__VALID_PHASE_HIGH, 0x38);
                WriteWord(SD_CONFIG__WOI_SD0, 0x0705);
                WriteWord(SD_CONFIG__INITIAL_PHASE_SD0, 0x0606);
                break;
            case 2:
                Write8(PHASECAL_CONFIG__TIMEOUT_MACROP, 0x0A);
                Write8(RANGE_CONFIG__VCSEL_PERIOD_A, 0x0F);
                Write8(RANGE_CONFIG__VCSEL_PERIOD_B, 0x0D);
                Write8(RANGE_CONFIG__VALID_PHASE_HIGH, 0xB8);
                WriteWord(SD_CONFIG__WOI_SD0, 0x0F0D);
                WriteWord(SD_CONFIG__INITIAL_PHASE_SD0, 0x0E0E);
                break;
            default: break;
        }
        SetTimingBudgetInMs(TB);
    }


    uint16_t GetDistanceMode()
    {
        uint8_t TempDM;

        TempDM = Read8(PHASECAL_CONFIG__TIMEOUT_MACROP);
        if(TempDM == 0x14)
            return 1;
        if(TempDM == 0x0A)
            return 2;
    }

    void SetInterMeasurementInMs(uint16_t InterMeasMs)
    {
        uint16_t ClockPLL;

        ClockPLL = Read8(RESULT__OSC_CALIBRATE_VAL);
        ClockPLL = ClockPLL & 0x3FF;
        WriteDWord(SYSTEM__INTERMEASUREMENT_PERIOD,
                   (uint32_t)(ClockPLL * InterMeasMs * 1.075));
    }

    uint16_t GetInterMeasurementInMs()
    {
        uint16_t ClockPLL;
        uint32_t tmp;
        uint16_t pIM;

        tmp      = ReadDWord(SYSTEM__INTERMEASUREMENT_PERIOD);
        pIM      = (uint16_t)tmp;
        ClockPLL = Read8(RESULT__OSC_CALIBRATE_VAL);
        ClockPLL = ClockPLL & 0x3FF;
        pIM      = (uint16_t)(pIM / (ClockPLL * 1.065));

        return pIM;
    }


    uint8_t BootState() { return Read8(FIRMWARE__SYSTEM_STATUS); }


    uint16_t GetSensorId() { return Read8(IDENTIFICATION__MODEL_ID); }

    uint16_t GetSignalPerSpad()
    {
        uint16_t SpNb = 1, signal;

        signal = Read8(
            RESULT__PEAK_SIGNAL_COUNT_RATE_CROSSTALK_CORRECTED_MCPS_SD0);
        SpNb = Read8(RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0);
        return (uint16_t)(2000.0 * signal / SpNb);
    }


    uint16_t GetAmbientPerSpad()
    {
        uint16_t AmbientRate, SpNb = 1;

        AmbientRate = Read8(RESULT__AMBIENT_COUNT_RATE_MCPS_SD);
        SpNb        = Read8(RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0);
        return (uint16_t)(2000.0 * AmbientRate / SpNb);
    }


    uint16_t GetSignalRate()
    {
        uint16_t tmp;

        tmp = Read8(
            RESULT__PEAK_SIGNAL_COUNT_RATE_CROSSTALK_CORRECTED_MCPS_SD0);
        return 8 * tmp;
    }


    uint16_t GetSpadNb()
    {
        uint16_t tmp;

        tmp = Read8(RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0);
        return tmp >> 8;
    }


    uint16_t GetAmbientRate()
    {
        uint16_t tmp;

        tmp = Read8(RESULT__AMBIENT_COUNT_RATE_MCPS_SD);
        return tmp * 8;
    }


    uint8_t GetRangeStatus()
    {
        uint8_t RgSt;

        RgSt = Read8(RESULT__RANGE_STATUS);
        RgSt = RgSt & 0x1F;
        switch(RgSt)
        {
            case 9: return 0;
            case 6: return 1;
            case 4: return 2;
            case 8: return 3;
            case 5: return 4;
            case 3: return 5;
            case 19: return 6;
            case 7: return 7;
            case 12: return 9;
            case 18: return 10;
            case 22: return 11;
            case 23: return 12;
            case 13: return 13;
            default: return 255;
        }
        return 255;
    }


    void SetOffset(int16_t OffsetValue)
    {
        int16_t Temp;

        Temp = (OffsetValue * 4);
        Temp = (uint16_t)WriteWord(ALGO__PART_TO_PART_RANGE_OFFSET_MM);
        WriteWord(MM_CONFIG__INNER_OFFSET_MM, 0x0);
        WriteWord(MM_CONFIG__OUTER_OFFSET_MM, 0x0);
    }


    uint16_t GetOffset()
    {
        uint16_t Temp;

        Temp = Read8(ALGO__PART_TO_PART_RANGE_OFFSET_MM);
        Temp = Temp << 3;
        return Temp / 32;
    }

    void SetXtalk(uint16_t XtalkValue)
    {
        /* XTalkValue in count per second to avoid float type */
        WriteWord(ALGO__CROSSTALK_COMPENSATION_X_PLANE_GRADIENT_KCPS, 0x0000);
        WriteWord(ALGO__CROSSTALK_COMPENSATION_Y_PLANE_GRADIENT_KCPS, 0x0000);

        /* * << 9 (7.9 format) and /1000 to convert cps to kpcs */
        WriteWord(ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS,
                  (XtalkValue << 9) / 1000);
    }


    uint16_t GetXtalk(uint16_t *xtalk)
    {
        uint16_t tmp;

        tmp = Read8(ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS);
        /* * 1000 to convert kcps to cps and >> 9 (7.9 format) */
        return (tmp * 1000) >> 9;
    }


    void SetDistanceThreshold(uint16_t ThreshLow,
                              uint16_t ThreshHigh,
                              uint8_t  Window,
                              uint8_t  IntOnNoTarget)
    {
        uint8_t Temp = 0;

        Temp = Read8(SYSTEM__INTERRUPT_CONFIG_GPIO);
        Temp = Temp & 0x47;
        if(IntOnNoTarget == 0)
        {
            Write8(SYSTEM__INTERRUPT_CONFIG_GPIO, (Temp | (Window & 0x07)));
        }
        else
        {
            Write8(SYSTEM__INTERRUPT_CONFIG_GPIO,
                   ((Temp | (Window & 0x07)) | 0x40));
        }
        WriteWord(SYSTEM__THRESH_HIGH, ThreshHigh);
        WriteWord(SYSTEM__THRESH_LOW, ThreshLow);
    }


    uint16_t GetDistanceThresholdWindow()
    {
        uint8_t tmp;
        tmp = Read8(SYSTEM__INTERRUPT_CONFIG_GPIO);
        return (uint16_t)(tmp & 0x7);
    }


    uint16_t GetDistanceThresholdLow() { return Read8(SYSTEM__THRESH_LOW); }

    uint16_t GetDistanceThresholdHigh() { return Read8(SYSTEM__THRESH_HIGH); }

    void SetROI(uint16_t X, uint16_t Y)
    {
        uint8_t OpticalCenter;

        OpticalCenter = Read8(ROI_CONFIG__MODE_ROI_CENTRE_SPAD);
        if(X > 16)
            X = 16;
        if(Y > 16)
            Y = 16;
        if(X > 10 || Y > 10)
        {
            OpticalCenter = 199;
        }
        Write8(ROI_CONFIG__USER_ROI_CENTRE_SPAD, OpticalCenter);
        Write8(ROI_CONFIG__USER_ROI_REQUESTED_GLOBAL_XY_SIZE,
               (Y - 1) << 4 | (X - 1));
    }

    void GetROI_XY(uint16_t *ROI_X, uint16_t *ROI_Y)
    {
        uint8_t tmp;

        tmp    = Read8(ROI_CONFIG__USER_ROI_REQUESTED_GLOBAL_XY_SIZE);
        *ROI_X = ((uint16_t)tmp & 0x0F) + 1;
        *ROI_Y = (((uint16_t)tmp & 0xF0) >> 4) + 1;
    }

    void SetROICenter(uint8_t ROICenter)
    {
        Write8(ROI_CONFIG__USER_ROI_CENTRE_SPAD, ROICenter);
    }

    uint8_t GetROICenter() { return Read8(ROI_CONFIG__USER_ROI_CENTRE_SPAD); }

    void SetSignalThreshold(uint16_t Signal)
    {
        WriteWord(RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS, Signal >> 3);
    }

    uint16_t GetSignalThreshold(uint16_t *signal)
    {
        return Read8(RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS);
    }

    void SetSigmaThreshold(uint16_t Sigma)
    {
        if(Sigma > (0xFFFF >> 2))
        {
            return;
        }
        /* 16 bits register 14.2 format */
        WriteWord(RANGE_CONFIG__SIGMA_THRESH, Sigma << 2);
    }

    uint16_t GetSigmaThreshold()
    {
        uint16_t tmp;

        tmp = Read8(RANGE_CONFIG__SIGMA_THRESH);
        return tmp >> 2;
    }

    void StartTemperatureUpdate()
    {
        uint8_t tmp = 0;

        Write8(VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND, 0x81); /* full VHV */
        Write8(0x0B, 0x92);
        StartRanging();
        while(tmp == 0)
        {
            tmp = CheckForDataReady();
        }
        tmp = 0;
        ClearInterrupt();
        StopRanging();
        Write8(VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND,
               0x09);    /* two bounds VHV */
        Write8(0x0B, 0); /* start VHV from the previous temperature */
    }

    /* calibration.h functions */

    uint16_t CalibrateOffset(uint16_t TargetDistInMm)
    {
        uint8_t  i               = 0, tmp;
        int16_t  AverageDistance = 0;
        uint16_t distance, offset;


        WriteWord(ALGO__PART_TO_PART_RANGE_OFFSET_MM, 0x0);
        WriteWord(MM_CONFIG__INNER_OFFSET_MM, 0x0);
        WriteWord(MM_CONFIG__OUTER_OFFSET_MM, 0x0);
        StartRanging(); /* Enable VL53L1X sensor */
        for(i = 0; i < 50; i++)
        {
            while(tmp == 0)
            {
                tmp = CheckForDataReady();
            }
            tmp      = 0;
            distance = GetDistance();
            ClearInterrupt();
            AverageDistance = AverageDistance + distance;
        }
        StopRanging();
        AverageDistance = AverageDistance / 50;
        offset          = TargetDistInMm - AverageDistance;
        WriteWord(ALGO__PART_TO_PART_RANGE_OFFSET_MM, offset * 4);
        return offset;
    }


    uint16_t CalibrateXtalk(uint16_t TargetDistInMm)
    {
        uint8_t  i, tmp = 0;
        float    AverageSignalRate = 0;
        float    AverageDistance   = 0;
        float    AverageSpadNb     = 0;
        uint16_t distance          = 0, spadNum;
        uint16_t sr;
        uint16_t xtalk;

        WriteWord(0x0016, 0);
        StartRanging();
        for(i = 0; i < 50; i++)
        {
            while(tmp == 0)
            {
                tmp = CheckForDataReady();
            }
            tmp      = 0;
            sr       = GetSignalRate();
            distance = GetDistance();
            ClearInterrupt();
            AverageDistance   = AverageDistance + distance;
            spadNum           = GetSpadNb();
            AverageSpadNb     = AverageSpadNb + spadNum;
            AverageSignalRate = AverageSignalRate + sr;
        }
        StopRanging();
        AverageDistance   = AverageDistance / 50;
        AverageSpadNb     = AverageSpadNb / 50;
        AverageSignalRate = AverageSignalRate / 50;
        /* Calculate Xtalk value */
        xtalk = (uint16_t)(
            512 * (AverageSignalRate * (1 - (AverageDistance / TargetDistInMm)))
            / AverageSpadNb);
        WriteWord(0x0016, xtalk);

        return xtalk;
    }

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
        0x00 /* 0x87 : start ranging, use StartRanging() or StopRanging(), If you want an automatic start after init() call, put 0x40 in location 0x87 */
    };

}; // namespace daisy
/** @} */

using Vl53l1xI2C = Vl53l1x<Vl53l1xI2CTransport>;
} // namespace daisy
#endif
