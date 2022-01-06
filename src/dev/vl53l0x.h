#pragma once
#ifndef DSY_VL53L0X_H
#define DSY_VL53L0X_H

/* Defines */
#define VL53L0X_SETPARAMETERFIELD(Dev, field, value) \
    PALDevDataSet(Dev, CurrentParameters.field, value)

#define VL53L0X_GETPARAMETERFIELD(Dev, field, variable) \
    variable = PALDevDataGet(Dev, CurrentParameters).field

#define VL53L0X_SETARRAYPARAMETERFIELD(Dev, field, index, value) \
    PALDevDataSet(Dev, CurrentParameters.field[index], value)

#define VL53L0X_GETARRAYPARAMETERFIELD(Dev, field, index, variable) \
    variable = PALDevDataGet(Dev, CurrentParameters).field[index]

#define VL53L0X_FIXPOINT1616TOFIXPOINT97(Value) \
    (uint16_t)((Value >> 9) & 0xFFFF)

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** I2C Transport for VL53L1X */
class Vl53l0xI2CTransport
{
  public:
    Vl53l0xI2CTransport() {}
    ~Vl53l0xI2CTransport() {}

    struct Config
    {
        I2CHandle::Config::Peripheral periph;
        I2CHandle::Config::Speed      speed;
        Pin                           scl;
        Pin                           sda;

        uint8_t dev_addr;

        Config()
        {
            periph = I2CHandle::Config::Peripheral::I2C_1;
            speed  = I2CHandle::Config::Speed::I2C_400KHZ;

            scl = Pin(PORTB, 8);
            sda = Pin(PORTB, 9);

            dev_addr = VL53L0X_I2C_ADDR;
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

        error_ = false;

        error_ |= I2CHandle::Result::OK != i2c_.Init(i2c_config);
    }

    void Write(uint8_t *data, uint16_t size)
    {
        I2CHandle::Result res
            = i2c_.TransmitBlocking(dev_addr_, data, size, 10);
        error_ |= (res != I2CHandle::\result::OK);
    }

    void Read(uint8_t *data, uint16_t size)
    {
        I2CHandle::Result res = i2c_.ReceiveBlocking(dev_addr_, data, size, 10);
        error_ |= (res != I2CHandle::Result::OK);
    }

    void Write8(uint8_t reg, uint8_t val)
    {
        uint8_t buff[2] = {reg, val};
        Write(buff, 2);
    }

    uint8_t Read8(uint8_t reg)
    {
        uint8_t ret;

        Write(reg);
        Read(ret);
        return ret;
    }
    /** \return true if error occured since last check */
    bool GetError()
    {
        bool tmp = error_;
        error_   = false;
        return error_;
    }

  private:
    I2CHandle i2c_;
    uint8_t   dev_addr_;
    bool      error_;
};


/** Device support for VL53L0X Time of Flight Sensor
    \author beserge
    \date December 2021
*/
template <typename Transport>
class Vl53l0x
{
  public:
    Vl53l0x() {}
    ~Vl53l0x() {}

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

    /** Initialize the VL53L0X device
        \param config Configuration settings
    */
    Result Init(Config config)
    {
        uint32_t refSpadCount;
        uint8_t  isApertureSpads;
        uint8_t  VhvSettings;
        uint8_t  PhaseCal;

        config_ = config;
        transport_.Init(config_.transport_config);

        VL53L0X_DataInit(&MyDevice); // Data initialization

        VL53L0X_GetDeviceInfo(&MyDevice, &DeviceInfo);

        if((DeviceInfo.ProductRevisionMajor != 1)
           || (DeviceInfo.ProductRevisionMinor != 1))
        {
            return ERR;
        }

        VL53L0X_StaticInit(pMyDevice); // Device Initialization

        VL53L0X_PerformRefSpadManagement(
            pMyDevice,
            &refSpadCount,
            &isApertureSpads); // Device Initialization

        Status
            = VL53L0X_PerformRefCalibration(pMyDevice,
                                            &VhvSettings,
                                            &PhaseCal); // Device Initialization

        // no need to do this when we use VL53L0X_PerformSingleRangingMeasurement
        VL53L0X_SetDeviceMode(
            pMyDevice,
            VL53L0X_DEVICEMODE_SINGLE_RANGING); // Setup in single ranging mode

        // call off to the config function to do the last part of configuration.
        ConfigSensor(vl_config);

        return GetTransportError();
    }

    uint8_t Read8(uint8_t reg) { return transport_.Read8(reg); }

    void Write8(uint8_t reg, uint8_t val) { transport_.Write8(reg, val); }

    void Update8(uint8_t reg, uint8_t AndData, uint8_t OrData)
    {
        uint8_t data = Read8(reg);
        data         = (data & AndData) | OrData;
        Write8(reg, data);
    }

    Result GetTransportError() { return transport_.GetError() ? ERR : OK; }

    /**  Configure the sensor for one of the ways the example ST sketches configure the sensors for different usages.
        \param  vl_config Which configureation you are trying to configure for
            It should be one of the following
            VL53L0X_SENSE_DEFAULT
            VL53L0X_SENSE_LONG_RANGE
            VL53L0X_SENSE_HIGH_SPEED,
            VL53L0X_SENSE_HIGH_ACCURACY
    */
    void ConfigSensor(VL53L0X_Sense_config_t vl_config)
    {
        // All of them appear to configure a few things

        // Enable/Disable Sigma and Signal check
        VL53L0X_SetLimitCheckEnable(
            pMyDevice, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
        VL53L0X_SetLimitCheckEnable(
            pMyDevice, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);

        switch(vl_config)
        {
            case VL53L0X_SENSE_DEFAULT:
                // Taken directly from SDK vl5310x_SingleRanging_example.c
                VL53L0X_SetLimitCheckEnable(
                    pMyDevice, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);

                VL53L0X_SetLimitCheckValue(
                    pMyDevice,
                    VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                    (FixPoint1616_t)(1.5 * 0.023 * 65536));
                break;
            case VL53L0X_SENSE_LONG_RANGE:
                VL53L0X_SetLimitCheckValue(
                    pMyDevice,
                    VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
                    (FixPoint1616_t)(0.1 * 65536));
                VL53L0X_SetLimitCheckValue(
                    pMyDevice,
                    VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
                    (FixPoint1616_t)(60 * 65536));
                VL53L0X_SetMeasurementTimingBudgetMicroSeconds(pMyDevice,
                                                               33000);

                VL53L0X_SetVcselPulsePeriod(
                    pMyDevice, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);
                VL53L0X_SetVcselPulsePeriod(
                    pMyDevice, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);
                break;
            case VL53L0X_SENSE_HIGH_SPEED:
                VL53L0X_SetLimitCheckValue(
                    pMyDevice,
                    VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
                    (FixPoint1616_t)(0.25 * 65536));
                VL53L0X_SetLimitCheckValue(
                    pMyDevice,
                    VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
                    (FixPoint1616_t)(32 * 65536));
                VL53L0X_SetMeasurementTimingBudgetMicroSeconds(pMyDevice,
                                                               30000);
                break;
            case VL53L0X_SENSE_HIGH_ACCURACY:
                // increase timing budget to 200 ms
                SetLimitCheckValue(VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
                                   (FixPoint1616_t)(0.25 * 65536));
                SetLimitCheckValue(VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
                                   (FixPoint1616_t)(18 * 65536));
                setMeasurementTimingBudgetMicroSeconds(200000);

                // Not sure about ignore threshold, try turnning it off...
                VL53L0X_SetLimitCheckEnable(
                    pMyDevice, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 0);
                break;
        }
    }

    /**  get a ranging measurement from the device
        \param  RangingMeasurementData the pointer to the struct the data will be stored in
    */
    Result GetSingleRangingMeasurement(
        VL53L0X_RangingMeasurementData_t *RangingMeasurementData)
    {
        FixPoint1616_t LimitCheckCurrent;

        //Step  4 : Test ranging mode
        VL53L0X_PerformSingleRangingMeasurement(pMyDevice,
                                                RangingMeasurementData);
    }

    /**  Single shot ranging.
        \return Distance in millimeters if valid
    */
    uint16_t ReadRange()
    {
        GetSingleRangingMeasurement(&measure, false);

        return measure.RangeMilliMeter;
    }

    /**  Start a range operation */
    bool StartRange()
    {
        // first lets set the device in SINGLE_Ranging mode
        VL53L0X_SetDeviceMode(pMyDevice, VL53L0X_DEVICEMODE_SINGLE_RANGING);

        // Lets start up the measurement
        VL53L0X_StartMeasurement(pMyDevice);
    }

    /**  Checks to see if a range operation has completed
        \return true if range operation completed or an error has happened
    */
    bool IsRangeComplete()
    {
        uint8_t NewDataReady = 0;
        VL53L0X_GetMeasurementDataReady(pMyDevice, &NewDataReady);
        Result Status = GetTransportError();
        return ((Status != OK) || (NewDataReady == 1));
    }

    /**  Wait until Range operation has completed. */
    void WaitRangeComplete()
    {
        VL53L0X_measurement_poll_for_completion(pMyDevice);
    }

    /**  Return the range in mm for the last operation.
        \return Range in mm.
    */
    uint16_t ReadRangeResult()
    {
        VL53L0X_RangingMeasurementData_t measure; // keep our own private copy

        VL53L0X_GetRangingMeasurementData(pMyDevice, &measure);
        _rangeStatus = measure.RangeStatus;
        VL53L0X_ClearInterruptMask(pMyDevice, 0);
        Result Status = GetTransportError();

        if((Status == OK) && (_rangeStatus != 4))
            return measure.RangeMilliMeter;
    }

    /**  Start a continuous range operation
        \param period_ms inter measurement period in milliseconds
    */
    void StartRangeContinuous(uint16_t period_ms)
    {
        // first lets set the device in SINGLE_Ranging mode
        VL53L0X_SetDeviceMode(pMyDevice,
                              VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING);

        VL53L0X_SetInterMeasurementPeriodMilliSeconds(pMyDevice, period_ms);

        // Lets start up the measurement
        VL53L0X_StartMeasurement(pMyDevice);
    }

    /**  Stop a continuous ranging operation */
    void StopRangeContinuous()
    {
        VL53L0X_StopMeasurement(pMyDevice);

        // lets wait until that completes.
        uint32_t StopCompleted = 0;
        uint32_t LoopNb        = 0;

        // Wait until it finished, use timeout to avoid deadlock
        do
        {
            VL53L0X_GetStopCompletedStatus(pMyDevice, &StopCompleted);
            if(StopCompleted == 0x00)
            {
                break;
            }
            LoopNb = LoopNb + 1;
            VL53L0X_PollingDelay(pMyDevice);

        } while(LoopNb < VL53L0X_DEFAULT_MAX_LOOP);

        if(LoopNb >= VL53L0X_DEFAULT_MAX_LOOP)
            return;


        VL53L0X_ClearInterruptMask(
            pMyDevice, VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY);
    }


    /**  Wrapper to ST library code to budget how long a measurement should take
        \param  budget_us the new budget
    */
    void SetMeasurementTimingBudgetMicroSeconds(uint32_t budget_us)
    {
        VL53L0X_SetMeasurementTimingBudgetMicroSeconds(pMyDevice, budget_us);
    }

    /**  Wrapper to ST library code to budget how long a measurement should take
        \return the current budget time in microseconds.
    */
    uint32_t GetMeasurementTimingBudgetMicroSeconds()
    {
        uint32_t budget_us;
        VL53L0X_GetMeasurementTimingBudgetMicroSeconds(pMyDevice, &budget_us);
        return budget_us;
    }

    /** Sets the VCSEL pulse period.
        \param   VcselPeriodType VCSEL period identifier (pre-range|final).
        \param   VCSELPulsePeriod VCSEL period value
    */
    void SetVcselPulsePeriod(VL53L0X_VcselPeriod VcselPeriodType,
                             uint8_t             VCSELPulsePeriod)
    {
        VL53L0X_SetVcselPulsePeriod(
            pMyDevice, VcselPeriodType, VCSELPulsePeriod);
    }

    /** Gets the VCSEL pulse period.
        \param   VcselPeriodType VCSEL period identifier (pre-range|final).
        \return the current pulse peried for the given type.
    */
    uint8_t GetVcselPulsePeriod(VL53L0X_VcselPeriod VcselPeriodType)
    {
        uint8_t cur_period;
        VL53L0X_GetVcselPulsePeriod(pMyDevice, VcselPeriodType, &cur_period);
        return cur_period;
    }

    /**  Enable/Disable a specific limit check
        \param   LimitCheckId Limit Check ID (0<= LimitCheckId < VL53L0X_GetNumberOfLimitCheck() ).
        \param   LimitCheckEnable if 1 the check limit corresponding to LimitCheckId is Enabled if 0 the check limit corresponding to LimitCheckId is disabled
    */
    void SetLimitCheckEnable(uint16_t LimitCheckId, uint8_t LimitCheckEnable)
    {
        VL53L0X_SetLimitCheckEnable(pMyDevice, LimitCheckId, LimitCheckEnable);
    }

    /**  Get specific limit check enable state
        \param   LimitCheckId Limit Check ID (0<= LimitCheckId < VL53L0X_GetNumberOfLimitCheck() ).
        \return  current state of limit enabled
    */
    uint8_t GetLimitCheckEnable(uint16_t LimitCheckId)
    {
        uint8_t cur_limit;
        VL53L0X_GetLimitCheckEnable(pMyDevice, LimitCheckId, &cur_limit);
        return cur_limit;
    }

    /**  Set a specific limit check value
        \param  LimitCheckId  Limit Check ID (0<= LimitCheckId < VL53L0X_GetNumberOfLimitCheck() ).
        \param  LimitCheckValue  Limit Check Value
    */
    void SetLimitCheckValue(uint16_t       LimitCheckId,
                            FixPoint1616_t LimitCheckValue)
    {
        VL53L0X_SetLimitCheckValue(pMyDevice, LimitCheckId, LimitCheckValue);
    }

    /**  Get a specific limit check value
        \param   LimitCheckId Limit Check ID (0<= LimitCheckId < VL53L0X_GetNumberOfLimitCheck() ).
        \return  limit check value in FixPoint1616
     */
    FixPoint1616_t GetLimitCheckValue(uint16_t LimitCheckId)
    {
        FixPoint1616_t LimitCheckValue;
        VL53L0X_GetLimitCheckValue(pMyDevice, LimitCheckId, &LimitCheckValue);
        return LimitCheckValue;
    }

    void VL53L0X_SetLimitCheckEnable(VL53L0X_DEV Dev,
                                     uint16_t    LimitCheckId,
                                     uint8_t     LimitCheckEnable)
    {
        FixPoint1616_t TempFix1616         = 0;
        uint8_t        LimitCheckEnableInt = 0;
        uint8_t        LimitCheckDisable   = 0;
        uint8_t        Temp8;

        if(LimitCheckId >= VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS)
        {
            return;
        }

        if(LimitCheckEnable == 0)
        {
            TempFix1616         = 0;
            LimitCheckEnableInt = 0;
            LimitCheckDisable   = 1;
        }
        else
        {
            VL53L0X_GETARRAYPARAMETERFIELD(
                Dev, LimitChecksValue, LimitCheckId, TempFix1616);
            LimitCheckDisable = 0;
            /* this to be sure to have either 0 or 1 */
            LimitCheckEnableInt = 1;
        }

        switch(LimitCheckId)
        {
            case VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE:
                /* internal computation: */
                VL53L0X_SETARRAYPARAMETERFIELD(
                    Dev,
                    LimitChecksEnable,
                    VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
                    LimitCheckEnableInt);
                break;

            case VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE:
                VL53L0X_WrWord(
                    Dev,
                    VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT,
                    VL53L0X_FIXPOINT1616TOFIXPOINT97(TempFix1616));
                break;

            case VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP:
                /* internal computation: */
                VL53L0X_SETARRAYPARAMETERFIELD(
                    Dev,
                    LimitChecksEnable,
                    VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP,
                    LimitCheckEnableInt);
                break;

            case VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD:
                /* internal computation: */
                VL53L0X_SETARRAYPARAMETERFIELD(
                    Dev,
                    LimitChecksEnable,
                    VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                    LimitCheckEnableInt);
                break;

            case VL53L0X_CHECKENABLE_SIGNAL_RATE_MSRC:
                Temp8 = (uint8_t)(LimitCheckDisable << 1);
                Update8(VL53L0X_REG_MSRC_CONFIG_CONTROL, 0xFE, Temp8);
                break;

            case VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE:
                Temp8 = (uint8_t)(LimitCheckDisable << 4);
                Update8(VL53L0X_REG_MSRC_CONFIG_CONTROL, 0xEF, Temp8);
                break;
            default: return;
        }

        if(LimitCheckEnable == 0)
        {
            VL53L0X_SETARRAYPARAMETERFIELD(
                Dev, LimitChecksEnable, LimitCheckId, 0);
        }
        else
        {
            VL53L0X_SETARRAYPARAMETERFIELD(
                Dev, LimitChecksEnable, LimitCheckId, 1);
        }
    }

    void VL53L0X_SetLimitCheckValue(VL53L0X_DEV    Dev,
                                    uint16_t       LimitCheckId,
                                    FixPoint1616_t LimitCheckValue)
    {
        uint8_t Temp8;

        VL53L0X_GETARRAYPARAMETERFIELD(
            Dev, LimitChecksEnable, LimitCheckId, Temp8);

        if(Temp8 == 0)
        { /* disabled write only internal value */
            VL53L0X_SETARRAYPARAMETERFIELD(
                Dev, LimitChecksValue, LimitCheckId, LimitCheckValue);
        }
        else
        {
            switch(LimitCheckId)
            {
                case VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE:
                    /* internal computation: */
                    VL53L0X_SETARRAYPARAMETERFIELD(
                        Dev,
                        LimitChecksValue,
                        VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
                        LimitCheckValue);
                    break;

                case VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE:
                    VL53L0X_WrWord(
                        Dev,
                        VL53L0X_REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT,
                        VL53L0X_FIXPOINT1616TOFIXPOINT97(LimitCheckValue));
                    break;

                case VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP:
                    /* internal computation: */
                    VL53L0X_SETARRAYPARAMETERFIELD(
                        Dev,
                        LimitChecksValue,
                        VL53L0X_CHECKENABLE_SIGNAL_REF_CLIP,
                        LimitCheckValue);
                    break;

                case VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD:
                    /* internal computation: */
                    VL53L0X_SETARRAYPARAMETERFIELD(
                        Dev,
                        LimitChecksValue,
                        VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                        LimitCheckValue);
                    break;

                case VL53L0X_CHECKENABLE_SIGNAL_RATE_MSRC: // fall thru
                case VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE:
                    VL53L0X_WrWord(
                        Dev,
                        VL53L0X_REG_PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT,
                        VL53L0X_FIXPOINT1616TOFIXPOINT97(LimitCheckValue));
                    break;

                default: return;
            }

            VL53L0X_SETARRAYPARAMETERFIELD(
                Dev, LimitChecksValue, LimitCheckId, LimitCheckValue);
        }
    }

    void VL53L0X_SetDeviceMode(VL53L0X_DEV Dev, VL53L0X_DeviceModes DeviceMode)
    {
        VL53L0X_SETPARAMETERFIELD(Dev, DeviceMode, DeviceMode);
    }

    void VL53L0X_SetMeasurementTimingBudgetMicroSeconds(
        VL53L0X_DEV Dev,
        uint32_t    MeasurementTimingBudgetMicroSeconds)
    {
        VL53L0X_set_measurement_timing_budget_micro_seconds(
            Dev, MeasurementTimingBudgetMicroSeconds);
    }

    void VL53L0X_SetVcselPulsePeriod(VL53L0X_DEV         Dev,
                                     VL53L0X_VcselPeriod VcselPeriodType,
                                     uint8_t             VCSELPulsePeriodPCLK)
    {
        VL53L0X_set_vcsel_pulse_period(
            Dev, VcselPeriodType, VCSELPulsePeriodPCLK);
    }

    void VL53L0X_StartMeasurement(VL53L0X_DEV Dev)
    {
        VL53L0X_DeviceModes DeviceMode;
        uint8_t             Byte;
        uint8_t  StartStopByte = VL53L0X_REG_SYSRANGE_MODE_START_STOP;
        uint32_t LoopNb;

        /* Get Current DeviceMode */
        VL53L0X_GetDeviceMode(Dev, &DeviceMode);

        Write8(0x80, 0x01);
        Write8(0xFF, 0x01);
        Write8(0x00, 0x00);
        Write8(0x91, PALDevDataGet(Dev, StopVariable));
        Write8(0x00, 0x01);
        Write8(0xFF, 0x00);
        Write8(0x80, 0x00);

        switch(DeviceMode)
        {
            case VL53L0X_DEVICEMODE_SINGLE_RANGING:
                Write8(VL53L0X_REG_SYSRANGE_START, 0x01);

                Byte = StartStopByte;
                /* Wait until start bit has been cleared */
                LoopNb = 0;
                do
                {
                    if(LoopNb > 0)
                        Byte = Read8(VL53L0X_REG_SYSRANGE_START);
                    LoopNb = LoopNb + 1;
                } while(((Byte & StartStopByte) == StartStopByte)
                        && (LoopNb < VL53L0X_DEFAULT_MAX_LOOP));

                if(LoopNb >= VL53L0X_DEFAULT_MAX_LOOP)
                    return;

                break;
            case VL53L0X_DEVICEMODE_CONTINUOUS_RANGING:
                /* Back-to-back mode */

                /* Check if need to apply interrupt settings */
                VL53L0X_CheckAndLoadInterruptSettings(Dev, 1);

                Write8(VL53L0X_REG_SYSRANGE_START,
                       VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK);

                /* Set PAL State to Running */
                PALDevDataSet(Dev, PalState, VL53L0X_STATE_RUNNING);

                break;
            case VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING:
                /* Continuous mode */
                /* Check if need to apply interrupt settings */
                VL53L0X_CheckAndLoadInterruptSettings(Dev, 1);

                Write8(VL53L0X_REG_SYSRANGE_START,
                       VL53L0X_REG_SYSRANGE_MODE_TIMED);

                /* Set PAL State to Running */
                PALDevDataSet(Dev, PalState, VL53L0X_STATE_RUNNING);

                break;
            default: break;
        }
    }

    /* Group PAL Interrupt Functions */
    void VL53L0X_ClearInterruptMask(VL53L0X_DEV Dev, uint32_t InterruptMask)
    {
        uint8_t Byte;

        /* clear bit 0 range interrupt, bit 1 error interrupt */
        do
        {
            Write8(VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
            Write8(VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x00);
            Byte = Read8(VL53L0X_REG_RESULT_INTERRUPT_STATUS);
        } while(((Byte & 0x07) != 0x00) && (LoopCount < 3));
    }
}; // class
/** @} */

using Vl53l0xI2C = Vl53l0x<Vl53l0xI2CTransport>;
} // namespace daisy
#endif