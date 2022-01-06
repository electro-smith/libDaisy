#pragma once
#ifndef DSY_VL53L0X_H
#define DSY_VL53L0X_H

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

            dev_addr = ;
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
    bool Write(uint8_t *data, uint16_t size)
    {
        return I2CHandle::Result::OK
               != i2c_.TransmitBlocking(dev_addr_, data, size, 10);
    }

    /** \return Did the transaction error? i.e. Return true if error, false if ok */
    bool Read(uint8_t *data, uint16_t size)
    {
        // dev_addr_ may need >> 1...
        return I2CHandle::Result::OK
               != i2c_.ReceiveBlocking(dev_addr_, data, size, 10);
    }

  private:
    I2CHandle i2c_;
    uint8_t   dev_addr_;
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
        config_ = config;

        return GetTransportErr();
    }


    /**  Setups the I2C interface and hardware
        \param  i2c_addr Optional I2C address the sensor can be found on. Default is 0x29
        \param debug Optional debug flag. If true, debug information will print out via Serial.print during setup. Defaults to false.
        \param  i2c Optional I2C bus the sensor is located on. Default is Wire
        \param vl_config Sensor configuration
        \return True if device is set up, false on any failure
    */
    boolean begin(uint8_t                i2c_addr,
                  boolean                debug,
                  TwoWire *              i2c,
                  VL53L0X_Sense_config_t vl_config)
    {
        uint32_t refSpadCount;
        uint8_t  isApertureSpads;
        uint8_t  VhvSettings;
        uint8_t  PhaseCal;

        // Initialize Comms
        pMyDevice->I2cDevAddr      = VL53L0X_I2C_ADDR; // default
        pMyDevice->comms_type      = 1;
        pMyDevice->comms_speed_khz = 400;
        pMyDevice->i2c             = i2c;

        pMyDevice->i2c->begin(); // VL53L0X_i2c_init();

        // unclear if this is even needed:
        if(VL53L0X_IMPLEMENTATION_VER_MAJOR != VERSION_REQUIRED_MAJOR
           || VL53L0X_IMPLEMENTATION_VER_MINOR != VERSION_REQUIRED_MINOR
           || VL53L0X_IMPLEMENTATION_VER_SUB != VERSION_REQUIRED_BUILD)
        {
            if(debug)
            {
                Serial.println(F(
                    "Found " STR(VL53L0X_IMPLEMENTATION_VER_MAJOR) "." STR(VL53L0X_IMPLEMENTATION_VER_MINOR) "." STR(
                        VL53L0X_IMPLEMENTATION_VER_SUB) " rev " STR(VL53L0X_IMPLEMENTATION_VER_REVISION)));
                Serial.println(F("Requires " STR(VERSION_REQUIRED_MAJOR) "." STR(
                    VERSION_REQUIRED_MINOR) "." STR(VERSION_REQUIRED_BUILD)));
            }

            Status = VL53L0X_ERROR_NOT_SUPPORTED;

            return false;
        }

        Status = VL53L0X_DataInit(&MyDevice); // Data initialization

        if(!setAddress(i2c_addr))
        {
            return false;
        }

        Status = VL53L0X_GetDeviceInfo(&MyDevice, &DeviceInfo);

        if(Status == VL53L0X_ERROR_NONE)
        {
            if(debug)
            {
                Serial.println(F("VL53L0X Info:"));
                Serial.print(F("Device Name: "));
                Serial.print(DeviceInfo.Name);
                Serial.print(F(", Type: "));
                Serial.print(DeviceInfo.Type);
                Serial.print(F(", ID: "));
                Serial.println(DeviceInfo.ProductId);

                Serial.print(F("Rev Major: "));
                Serial.print(DeviceInfo.ProductRevisionMajor);
                Serial.print(F(", Minor: "));
                Serial.println(DeviceInfo.ProductRevisionMinor);
            }

            if((DeviceInfo.ProductRevisionMajor != 1)
               || (DeviceInfo.ProductRevisionMinor != 1))
            {
                if(debug)
                {
                    Serial.print(F("Error expected cut 1.1 but found "));
                    Serial.print(DeviceInfo.ProductRevisionMajor);
                    Serial.print(',');
                    Serial.println(DeviceInfo.ProductRevisionMinor);
                }

                Status = VL53L0X_ERROR_NOT_SUPPORTED;
            }
        }

        if(Status == VL53L0X_ERROR_NONE)
        {
            if(debug)
            {
                Serial.println(F("VL53L0X: StaticInit"));
            }

            Status = VL53L0X_StaticInit(pMyDevice); // Device Initialization
        }

        if(Status == VL53L0X_ERROR_NONE)
        {
            if(debug)
            {
                Serial.println(F("VL53L0X: PerformRefSpadManagement"));
            }

            Status = VL53L0X_PerformRefSpadManagement(
                pMyDevice,
                &refSpadCount,
                &isApertureSpads); // Device Initialization

            if(debug)
            {
                Serial.print(F("refSpadCount = "));
                Serial.print(refSpadCount);
                Serial.print(F(", isApertureSpads = "));
                Serial.println(isApertureSpads);
            }
        }

        if(Status == VL53L0X_ERROR_NONE)
        {
            if(debug)
            {
                Serial.println(F("VL53L0X: PerformRefCalibration"));
            }

            Status = VL53L0X_PerformRefCalibration(
                pMyDevice,
                &VhvSettings,
                &PhaseCal); // Device Initialization
        }

        if(Status == VL53L0X_ERROR_NONE)
        {
            // no need to do this when we use VL53L0X_PerformSingleRangingMeasurement
            if(debug)
            {
                Serial.println(F("VL53L0X: SetDeviceMode"));
            }

            Status = VL53L0X_SetDeviceMode(
                pMyDevice,
                VL53L0X_DEVICEMODE_SINGLE_RANGING); // Setup in single ranging mode
        }

        // call off to the config function to do the last part of configuration.
        if(Status == VL53L0X_ERROR_NONE)
        {
            configSensor(vl_config);
        }

        if(Status == VL53L0X_ERROR_NONE)
        {
            return true;
        }
        else
        {
            if(debug)
            {
                Serial.print(F("VL53L0X Error: "));
                Serial.println(Status);
            }

            return false;
        }
    }

    /**  Change the I2C address of the sensor
        \param  newAddr the new address to set the sensor to
        \return True if address was set successfully, False otherwise
    */
    boolean setAddress(uint8_t newAddr)
    {
        newAddr &= 0x7F;

        Status = VL53L0X_SetDeviceAddress(pMyDevice, newAddr * 2); // 7->8 bit

        delay(10);

        if(Status == VL53L0X_ERROR_NONE)
        {
            pMyDevice->I2cDevAddr = newAddr; // 7 bit addr
            return true;
        }
        return false;
    }

    /**  Configure the sensor for one of the ways the example ST sketches configure the sensors for different usages.
        \param  vl_config Which configureation you are trying to configure for
            It should be one of the following
            VL53L0X_SENSE_DEFAULT
            VL53L0X_SENSE_LONG_RANGE
            VL53L0X_SENSE_HIGH_SPEED,
            VL53L0X_SENSE_HIGH_ACCURACY
        \return True if address was set successfully, False otherwise
    */
    boolean configSensor(VL53L0X_Sense_config_t vl_config)
    {
        // All of them appear to configure a few things

        // Serial.print(F("VL53L0X: configSensor "));
        // Serial.println((int)vl_config, DEC);
        // Enable/Disable Sigma and Signal check
        Status = VL53L0X_SetLimitCheckEnable(
            pMyDevice, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);

        if(Status == VL53L0X_ERROR_NONE)
        {
            Status = VL53L0X_SetLimitCheckEnable(
                pMyDevice, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
        }

        if(Status != VL53L0X_ERROR_NONE)
            return false;

        switch(vl_config)
        {
            case VL53L0X_SENSE_DEFAULT:
                // Taken directly from SDK vl5310x_SingleRanging_example.c
                // Maybe should convert to helper functions but...
                // Serial.println("  VL53L0X_SENSE_DEFAULT");
                if(Status == VL53L0X_ERROR_NONE)
                {
                    Status = VL53L0X_SetLimitCheckEnable(
                        pMyDevice,
                        VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                        1);
                }

                if(Status == VL53L0X_ERROR_NONE)
                {
                    Status = VL53L0X_SetLimitCheckValue(
                        pMyDevice,
                        VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                        (FixPoint1616_t)(1.5 * 0.023 * 65536));
                }
                break;
            case VL53L0X_SENSE_LONG_RANGE:
                Serial.println("  VL53L0X_SENSE_LONG_RANGE");
                Status = VL53L0X_SetLimitCheckValue(
                    pMyDevice,
                    VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
                    (FixPoint1616_t)(0.1 * 65536));
                if(Status == VL53L0X_ERROR_NONE)
                {
                    Status = VL53L0X_SetLimitCheckValue(
                        pMyDevice,
                        VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
                        (FixPoint1616_t)(60 * 65536));
                }
                if(Status == VL53L0X_ERROR_NONE)
                {
                    Status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(
                        pMyDevice, 33000);
                }

                if(Status == VL53L0X_ERROR_NONE)
                {
                    Status = VL53L0X_SetVcselPulsePeriod(
                        pMyDevice, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);
                }
                if(Status == VL53L0X_ERROR_NONE)
                {
                    Status = VL53L0X_SetVcselPulsePeriod(
                        pMyDevice, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);
                }
                break;
            case VL53L0X_SENSE_HIGH_SPEED:
                // Serial.println("  VL53L0X_SENSE_HIGH_SPEED");
                Status = VL53L0X_SetLimitCheckValue(
                    pMyDevice,
                    VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
                    (FixPoint1616_t)(0.25 * 65536));
                if(Status == VL53L0X_ERROR_NONE)
                {
                    Status = VL53L0X_SetLimitCheckValue(
                        pMyDevice,
                        VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
                        (FixPoint1616_t)(32 * 65536));
                }
                if(Status == VL53L0X_ERROR_NONE)
                {
                    Status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(
                        pMyDevice, 30000);
                }
                break;
            case VL53L0X_SENSE_HIGH_ACCURACY:
                // increase timing budget to 200 ms

                if(Status == VL53L0X_ERROR_NONE)
                {
                    SetLimitCheckValue(
                        VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
                        (FixPoint1616_t)(0.25 * 65536));
                }
                if(Status == VL53L0X_ERROR_NONE)
                {
                    SetLimitCheckValue(VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
                                       (FixPoint1616_t)(18 * 65536));
                }
                if(Status == VL53L0X_ERROR_NONE)
                {
                    setMeasurementTimingBudgetMicroSeconds(200000);
                }
                // Not sure about ignore threshold, try turnning it off...
                if(Status == VL53L0X_ERROR_NONE)
                {
                    Status = VL53L0X_SetLimitCheckEnable(
                        pMyDevice,
                        VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                        0);
                }

                break;
        }

        return (Status == VL53L0X_ERROR_NONE);
    }

    /**  get a ranging measurement from the device
        \param  RangingMeasurementData the pointer to the struct the data will be stored in
        \param debug Optional debug flag. If true debug information will print via Serial.print during execution. Defaults to false.
        \return True if address was set successfully, False otherwise
    */
    VL53L0X_Error getSingleRangingMeasurement(
        VL53L0X_RangingMeasurementData_t *RangingMeasurementData,
        boolean                           debug)
    {
        VL53L0X_Error  Status = VL53L0X_ERROR_NONE;
        FixPoint1616_t LimitCheckCurrent;

        /*
   *  Step  4 : Test ranging mode
   */

        if(Status == VL53L0X_ERROR_NONE)
        {
            if(debug)
            {
                Serial.println(F("sVL53L0X: PerformSingleRangingMeasurement"));
            }
            Status = VL53L0X_PerformSingleRangingMeasurement(
                pMyDevice, RangingMeasurementData);

            if(debug)
            {
                printRangeStatus(RangingMeasurementData);
            }

            if(debug)
            {
                VL53L0X_GetLimitCheckCurrent(
                    pMyDevice,
                    VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                    &LimitCheckCurrent);

                Serial.print(F("RANGE IGNORE THRESHOLD: "));
                Serial.println((float)LimitCheckCurrent / 65536.0);

                Serial.print(F("Measured distance: "));
                Serial.println(RangingMeasurementData->RangeMilliMeter);
            }
        }

        return Status;
    }

    /**  print a ranging measurement out via Serial.print in a human-readable format
        \param pRangingMeasurementData a pointer to the ranging measurement data
    */
    void
    printRangeStatus(VL53L0X_RangingMeasurementData_t *pRangingMeasurementData)
    {
        char    buf[VL53L0X_MAX_STRING_LENGTH];
        uint8_t RangeStatus;

        /*
   * New Range Status: data is valid when pRangingMeasurementData->RangeStatus =
   * 0
   */

        RangeStatus = pRangingMeasurementData->RangeStatus;

        VL53L0X_GetRangeStatusString(RangeStatus, buf);

        Serial.print(F("Range Status: "));
        Serial.print(RangeStatus);
        Serial.print(F(" : "));
        Serial.println(buf);
    }

    /**  Single shot ranging. Be sure to check the return of readRangeStatus to before using the return value!
        \return Distance in millimeters if valid
    */
    uint16_t readRange(void)
    {
        VL53L0X_RangingMeasurementData_t measure; // keep our own private copy

        Status       = getSingleRangingMeasurement(&measure, false);
        _rangeStatus = measure.RangeStatus;

        if(Status == VL53L0X_ERROR_NONE)
            return measure.RangeMilliMeter;
        // Other status return something totally out of bounds...
        return 0xffff;
    }

    /**  Request ranging success/error message (retrieve after ranging)
        \return One of possible VL6180X_ERROR_* values
    */
    uint8_t readRangeStatus(void) { return _rangeStatus; }

    /**  Start a range operation
        \return true if range operation successfully started.
    */
    boolean startRange(void)
    {
        // This function will do a complete single ranging
        // Here we fix the mode!
        // first lets set the device in SINGLE_Ranging mode
        Status = VL53L0X_SetDeviceMode(pMyDevice,
                                       VL53L0X_DEVICEMODE_SINGLE_RANGING);

        if(Status == VL53L0X_ERROR_NONE)
        {
            // Lets start up the measurement
            Status = VL53L0X_StartMeasurement(pMyDevice);
        }
        return (Status == VL53L0X_ERROR_NONE);
    }

    /**  Checks to see if a range operation has completed
        \return true if range operation completed or an error has happened
    */
    boolean isRangeComplete(void)
    {
        uint8_t NewDataReady = 0;
        Status = VL53L0X_GetMeasurementDataReady(pMyDevice, &NewDataReady);
        return ((Status != VL53L0X_ERROR_NONE) || (NewDataReady == 1));
    }

    /**  Wait until Range operation has completed.
        \return true if range operation completed, false if error.
    */
    boolean waitRangeComplete(void)
    {
        Status = VL53L0X_measurement_poll_for_completion(pMyDevice);

        return (Status == VL53L0X_ERROR_NONE);
    }

    /**  Return the range in mm for the last operation.
        \return Range in mm.
    */
    uint16_t readRangeResult(void)
    {
        VL53L0X_RangingMeasurementData_t measure; // keep our own private copy

        Status       = VL53L0X_GetRangingMeasurementData(pMyDevice, &measure);
        _rangeStatus = measure.RangeStatus;
        if(Status == VL53L0X_ERROR_NONE)
            Status = VL53L0X_ClearInterruptMask(pMyDevice, 0);

        if((Status == VL53L0X_ERROR_NONE) && (_rangeStatus != 4))
            return measure.RangeMilliMeter;

        return 0xffff; // some out of range value
    }

    /**  Start a continuous range operation
        \param period_ms inter measurement period in milliseconds
        \return True if successful, false otherwise
    */
    boolean StartRangeContinuous(uint16_t period_ms)
    {
        // This function will do a complete single ranging
        // Here we fix the mode!
        // first lets set the device in SINGLE_Ranging mode
        Status = VL53L0X_SetDeviceMode(
            pMyDevice, VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING);

        if(Status == VL53L0X_ERROR_NONE)
        {
            Status = VL53L0X_SetInterMeasurementPeriodMilliSeconds(pMyDevice,
                                                                   period_ms);
        }

        if(Status == VL53L0X_ERROR_NONE)
        {
            // Lets start up the measurement
            Status = VL53L0X_StartMeasurement(pMyDevice);
        }
        return (Status == VL53L0X_ERROR_NONE);
    }

    /**  Stop a continuous ranging operation */
    void StopRangeContinuous(void)
    {
        Status = VL53L0X_StopMeasurement(pMyDevice);

        // lets wait until that completes.
        uint32_t StopCompleted = 0;
        uint32_t LoopNb;

        // Wait until it finished
        // use timeout to avoid deadlock
        if(Status == VL53L0X_ERROR_NONE)
        {
            LoopNb = 0;
            do
            {
                Status
                    = VL53L0X_GetStopCompletedStatus(pMyDevice, &StopCompleted);
                if((StopCompleted == 0x00) || Status != VL53L0X_ERROR_NONE)
                {
                    break;
                }
                LoopNb = LoopNb + 1;
                VL53L0X_PollingDelay(pMyDevice);
            } while(LoopNb < VL53L0X_DEFAULT_MAX_LOOP);

            if(LoopNb >= VL53L0X_DEFAULT_MAX_LOOP)
            {
                Status = VL53L0X_ERROR_TIME_OUT;
            }
        }

        if(Status == VL53L0X_ERROR_NONE)
        {
            Status = VL53L0X_ClearInterruptMask(
                pMyDevice, VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY);
        }
    }

    /**  Wrapper to ST library code to budget how long a measurement should take
        \param  budget_us the new budget
        \return True if success
    */
    boolean SetMeasurementTimingBudgetMicroSeconds(uint32_t budget_us)
    {
        Status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(pMyDevice,
                                                                budget_us);
        return (Status == VL53L0X_ERROR_NONE);
    }

    /**  Wrapper to ST library code to budget how long a measurement should take
        \return the current budget time in microseconds.
    */
    uint32_t GetMeasurementTimingBudgetMicroSeconds()
    {
        uint32_t budget_us;
        Status = VL53L0X_GetMeasurementTimingBudgetMicroSeconds(pMyDevice,
                                                                &budget_us);
        return (budget_us);
    }

    /** Sets the VCSEL pulse period.
        \param   VcselPeriodType VCSEL period identifier (pre-range|final).
        \param   VCSELPulsePeriod VCSEL period value
        \return True if success
    */
    boolean SetVcselPulsePeriod(VL53L0X_VcselPeriod VcselPeriodType,
                                uint8_t             VCSELPulsePeriod)
    {
        Status = VL53L0X_SetVcselPulsePeriod(
            pMyDevice, VcselPeriodType, VCSELPulsePeriod);
        return (Status == VL53L0X_ERROR_NONE);
    }

    /** Gets the VCSEL pulse period.
        \param   VcselPeriodType VCSEL period identifier (pre-range|final).
        \return the current pulse peried for the given type.
    */
    uint8_t GetVcselPulsePeriod(VL53L0X_VcselPeriod VcselPeriodType)
    {
        uint8_t cur_period;
        Status = VL53L0X_GetVcselPulsePeriod(
            pMyDevice, VcselPeriodType, &cur_period);
        return (cur_period);
    }

    /**  Enable/Disable a specific limit check
        \param   LimitCheckId Limit Check ID (0<= LimitCheckId < VL53L0X_GetNumberOfLimitCheck() ).
        \param   LimitCheckEnable if 1 the check limit corresponding to LimitCheckId is Enabled if 0 the check limit corresponding to LimitCheckId is disabled
        \return  true if succeeded
    */
    boolean SetLimitCheckEnable(uint16_t LimitCheckId, uint8_t LimitCheckEnable)
    {
        Status = VL53L0X_SetLimitCheckEnable(
            pMyDevice, LimitCheckId, LimitCheckEnable);
        return (Status == VL53L0X_ERROR_NONE);
    }

    /**  Get specific limit check enable state
        \param   LimitCheckId Limit Check ID (0<= LimitCheckId < VL53L0X_GetNumberOfLimitCheck() ).
        \return  current state of limit enabled
    */
    uint8_t GetLimitCheckEnable(uint16_t LimitCheckId)
    {
        uint8_t cur_limit;
        Status
            = VL53L0X_GetLimitCheckEnable(pMyDevice, LimitCheckId, &cur_limit);
        return (cur_limit);
    }

    /**  Set a specific limit check value
        \param  LimitCheckId  Limit Check ID (0<= LimitCheckId < VL53L0X_GetNumberOfLimitCheck() ).
        \param  LimitCheckValue  Limit Check Value
        \return  true if succeeded.
    */
    boolean SetLimitCheckValue(uint16_t       LimitCheckId,
                               FixPoint1616_t LimitCheckValue)
    {
        Status = VL53L0X_SetLimitCheckValue(
            pMyDevice, LimitCheckId, LimitCheckValue);
        return (Status == VL53L0X_ERROR_NONE);
    }

    /**  Get a specific limit check value
        \param   LimitCheckId Limit Check ID (0<= LimitCheckId < VL53L0X_GetNumberOfLimitCheck() ).
        \return  limit check value in FixPoint1616
     */
    FixPoint1616_t GetLimitCheckValue(uint16_t LimitCheckId)
    {
        FixPoint1616_t LimitCheckValue;
        Status = VL53L0X_GetLimitCheckValue(
            pMyDevice, LimitCheckId, &LimitCheckValue);
        return (LimitCheckValue);
    }

    Result VL53L0X_SetLimitCheckEnable(VL53L0X_DEV Dev,
                                       uint16_t    LimitCheckId,
                                       uint8_t     LimitCheckEnable)
    {
        VL53L0X_Error  Status              = VL53L0X_ERROR_NONE;
        FixPoint1616_t TempFix1616         = 0;
        uint8_t        LimitCheckEnableInt = 0;
        uint8_t        LimitCheckDisable   = 0;
        uint8_t        Temp8;

        if(LimitCheckId >= VL53L0X_CHECKENABLE_NUMBER_OF_CHECKS)
        {
            Return ERR;
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

                Status = VL53L0X_WrWord(
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

                Temp8  = (uint8_t)(LimitCheckDisable << 1);
                Status = VL53L0X_UpdateByte(
                    Dev, VL53L0X_REG_MSRC_CONFIG_CONTROL, 0xFE, Temp8);

                break;

            case VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE:

                Temp8  = (uint8_t)(LimitCheckDisable << 4);
                Status = VL53L0X_UpdateByte(
                    Dev, VL53L0X_REG_MSRC_CONFIG_CONTROL, 0xEF, Temp8);

                break;

            default: return ERR;
        }

        return OK;
    }

    if(Status == VL53L0X_ERROR_NONE)
    {
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

    LOG_FUNCTION_END(Status);
    return Status;
}

VL53L0X_Error
VL53L0X_SetLimitCheckValue(VL53L0X_DEV    Dev,
                           uint16_t       LimitCheckId,
                           FixPoint1616_t LimitCheckValue)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    uint8_t       Temp8;

    LOG_FUNCTION_START("");

    VL53L0X_GETARRAYPARAMETERFIELD(Dev, LimitChecksEnable, LimitCheckId, Temp8);

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

                Status = VL53L0X_WrWord(
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

            case VL53L0X_CHECKENABLE_SIGNAL_RATE_MSRC:
            case VL53L0X_CHECKENABLE_SIGNAL_RATE_PRE_RANGE:

                Status = VL53L0X_WrWord(
                    Dev,
                    VL53L0X_REG_PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT,
                    VL53L0X_FIXPOINT1616TOFIXPOINT97(LimitCheckValue));

                break;

            default: Status = VL53L0X_ERROR_INVALID_PARAMS;
        }

        if(Status == VL53L0X_ERROR_NONE)
        {
            VL53L0X_SETARRAYPARAMETERFIELD(
                Dev, LimitChecksValue, LimitCheckId, LimitCheckValue);
        }
    }

    LOG_FUNCTION_END(Status);
    return Status;
}

VL53L0X_Error VL53L0X_SetDeviceMode(VL53L0X_DEV         Dev,
                                    VL53L0X_DeviceModes DeviceMode)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;

    LOG_FUNCTION_START("%d", (int)DeviceMode);

    switch(DeviceMode)
    {
        case VL53L0X_DEVICEMODE_SINGLE_RANGING:
        case VL53L0X_DEVICEMODE_CONTINUOUS_RANGING:
        case VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING:
        case VL53L0X_DEVICEMODE_GPIO_DRIVE:
        case VL53L0X_DEVICEMODE_GPIO_OSC:
            /* Supported modes */
            VL53L0X_SETPARAMETERFIELD(Dev, DeviceMode, DeviceMode);
            break;
        default:
            /* Unsupported mode */
            Status = VL53L0X_ERROR_MODE_NOT_SUPPORTED;
    }

    LOG_FUNCTION_END(Status);
    return Status;
}

VL53L0X_Error VL53L0X_SetMeasurementTimingBudgetMicroSeconds(
    VL53L0X_DEV Dev,
    uint32_t    MeasurementTimingBudgetMicroSeconds)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    LOG_FUNCTION_START("");

    Status = VL53L0X_set_measurement_timing_budget_micro_seconds(
        Dev, MeasurementTimingBudgetMicroSeconds);

    LOG_FUNCTION_END(Status);

    return Status;
}

VL53L0X_Error VL53L0X_SetVcselPulsePeriod(VL53L0X_DEV         Dev,
                                          VL53L0X_VcselPeriod VcselPeriodType,
                                          uint8_t VCSELPulsePeriodPCLK)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    LOG_FUNCTION_START("");

    Status = VL53L0X_set_vcsel_pulse_period(
        Dev, VcselPeriodType, VCSELPulsePeriodPCLK);

    LOG_FUNCTION_END(Status);
    return Status;
}

VL53L0X_Error VL53L0X_StartMeasurement(VL53L0X_DEV Dev)
{
    VL53L0X_Error       Status = VL53L0X_ERROR_NONE;
    VL53L0X_DeviceModes DeviceMode;
    uint8_t             Byte;
    uint8_t             StartStopByte = VL53L0X_REG_SYSRANGE_MODE_START_STOP;
    uint32_t            LoopNb;
    LOG_FUNCTION_START("");

    /* Get Current DeviceMode */
    VL53L0X_GetDeviceMode(Dev, &DeviceMode);

    Status = VL53L0X_WrByte(Dev, 0x80, 0x01);
    Status = VL53L0X_WrByte(Dev, 0xFF, 0x01);
    Status = VL53L0X_WrByte(Dev, 0x00, 0x00);
    Status = VL53L0X_WrByte(Dev, 0x91, PALDevDataGet(Dev, StopVariable));
    Status = VL53L0X_WrByte(Dev, 0x00, 0x01);
    Status = VL53L0X_WrByte(Dev, 0xFF, 0x00);
    Status = VL53L0X_WrByte(Dev, 0x80, 0x00);

    switch(DeviceMode)
    {
        case VL53L0X_DEVICEMODE_SINGLE_RANGING:
            Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSRANGE_START, 0x01);

            Byte = StartStopByte;
            if(Status == VL53L0X_ERROR_NONE)
            {
                /* Wait until start bit has been cleared */
                LoopNb = 0;
                do
                {
                    if(LoopNb > 0)
                        Status = VL53L0X_RdByte(
                            Dev, VL53L0X_REG_SYSRANGE_START, &Byte);
                    LoopNb = LoopNb + 1;
                } while(((Byte & StartStopByte) == StartStopByte)
                        && (Status == VL53L0X_ERROR_NONE)
                        && (LoopNb < VL53L0X_DEFAULT_MAX_LOOP));

                if(LoopNb >= VL53L0X_DEFAULT_MAX_LOOP)
                    Status = VL53L0X_ERROR_TIME_OUT;
            }

            break;
        case VL53L0X_DEVICEMODE_CONTINUOUS_RANGING:
            /* Back-to-back mode */

            /* Check if need to apply interrupt settings */
            if(Status == VL53L0X_ERROR_NONE)
                Status = VL53L0X_CheckAndLoadInterruptSettings(Dev, 1);

            Status = VL53L0X_WrByte(Dev,
                                    VL53L0X_REG_SYSRANGE_START,
                                    VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK);
            if(Status == VL53L0X_ERROR_NONE)
            {
                /* Set PAL State to Running */
                PALDevDataSet(Dev, PalState, VL53L0X_STATE_RUNNING);
            }
            break;
        case VL53L0X_DEVICEMODE_CONTINUOUS_TIMED_RANGING:
            /* Continuous mode */
            /* Check if need to apply interrupt settings */
            if(Status == VL53L0X_ERROR_NONE)
                Status = VL53L0X_CheckAndLoadInterruptSettings(Dev, 1);

            Status = VL53L0X_WrByte(Dev,
                                    VL53L0X_REG_SYSRANGE_START,
                                    VL53L0X_REG_SYSRANGE_MODE_TIMED);

            if(Status == VL53L0X_ERROR_NONE)
            {
                /* Set PAL State to Running */
                PALDevDataSet(Dev, PalState, VL53L0X_STATE_RUNNING);
            }
            break;
        default:
            /* Selected mode not supported */
            Status = VL53L0X_ERROR_MODE_NOT_SUPPORTED;
    }

    LOG_FUNCTION_END(Status);
    return Status;
}

/* Group PAL Interrupt Functions */
VL53L0X_Error VL53L0X_ClearInterruptMask(VL53L0X_DEV Dev,
                                         uint32_t    InterruptMask)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    uint8_t       LoopCount;
    uint8_t       Byte;
    LOG_FUNCTION_START("");

    /* clear bit 0 range interrupt, bit 1 error interrupt */
    LoopCount = 0;
    do
    {
        Status = VL53L0X_WrByte(Dev, VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x01);
        Status |= VL53L0X_WrByte(Dev, VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR, 0x00);
        Status
            |= VL53L0X_RdByte(Dev, VL53L0X_REG_RESULT_INTERRUPT_STATUS, &Byte);
        LoopCount++;
    } while(((Byte & 0x07) != 0x00) && (LoopCount < 3)
            && (Status == VL53L0X_ERROR_NONE));

    if(LoopCount >= 3)
        Status = VL53L0X_ERROR_INTERRUPT_NOT_CLEARED;

    LOG_FUNCTION_END(Status);
    return Status;
}

}; // namespace daisy
/** @} */

using Vl53l0xI2C = Vl53l0x<Vl53l0xI2CTransport>;
} // namespace daisy
#endif