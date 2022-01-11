
/*!
 * @file Adafruit_VL53L0X.h

  This is a library for the Adafruit VL53L0X Sensor Breakout

  Designed specifically to work with the VL53L0X sensor from Adafruit
  ----> https://www.adafruit.com/products/3317

  These sensors use I2C to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#pragma once
#ifndef DSY_VL53L0X_H
#define DSY_VL53L0X_H

#include "sys/system.h"
#include "dev/vl53l0x/vl53l0x_api.h"

#define VL53L0X_I2C_ADDR 0x29 ///< Default sensor I2C address

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/**************************************************************************/
/**  Class that stores state and functions for interacting with VL53L0X
   time-of-flight sensor chips
*/
/**************************************************************************/
class Adafruit_VL53L0X
{
  public:
    Adafruit_VL53L0X() {}
    ~Adafruit_VL53L0X() {}

    /** Sensor configurations */
    enum VL53L0X_Sense_config_t
    {
        VL53L0X_SENSE_DEFAULT = 0,
        VL53L0X_SENSE_LONG_RANGE,
        VL53L0X_SENSE_HIGH_SPEED,
        VL53L0X_SENSE_HIGH_ACCURACY
    };

    struct Config
    {
        VL53L0X_Sense_config_t vl_config;

        I2CHandle::Config::Peripheral periph;
        I2CHandle::Config::Speed      speed;
        Pin                           scl;
        Pin                           sda;

        uint8_t dev_addr;

        Config()
        {
            vl_config = VL53L0X_SENSE_DEFAULT;

            periph = I2CHandle::Config::Peripheral::I2C_1;
            speed  = I2CHandle::Config::Speed::I2C_400KHZ;

            scl = Pin(PORTB, 8);
            sda = Pin(PORTB, 9);

            dev_addr = VL53L0X_I2C_ADDR;
        }
    };

    enum Result
    {
        OK = 0,
        ERR
    };

    /** Initialize the VL53L0X device
        \param config Configuration settings
    */
    Result Init(Config config);

    bool setAddress(uint8_t newAddr);

    /** Get the last error code
        \return VL53L0X_Error 0 OK, anything else ERR
    */
    VL53L0X_Error GetError() { return Status; }

    /** Perform a srm and store for getters to use later */
    void Process() { measurement_data_ = getSingleRangingMeasurement(); }

    /** 32-bit time stamp. */
    uint32_t GetTimeStamp() { return measurement_data_.TimeStamp; }

    /** Give the Measurement time needed by the device to do the measurement.*/
    uint32_t GetMeasurementTimeUsec()
    {
        return measurement_data_.MeasurementTimeUsec;
    }

    /** range distance in millimeter. */
    uint16_t GetRangeMilliMeter() { return measurement_data_.RangeMilliMeter; }

    /** Tells what is the maximum detection distance of the device in current setup and environment conditions 
        (Filled when applicable) 
    */
    uint16_t GetRangeDMaxMilliMeter()
    {
        return measurement_data_.RangeDMaxMilliMeter;
    }

    /** Return signal rate (MCPS) this is a 16.16 fix point value, which is effectively a measure of target reflectance. */
    FixPoint1616_t GetSignalRateRtnMegaCps()
    {
        return measurement_data_.SignalRateRtnMegaCps;
    }

    /** Return ambient rate (MCPS) this is a 16.16 fix point value, which is effectively a measure of the ambient light.*/
    FixPoint1616_t GetAmbientRateRtnMegaCps()
    {
        return measurement_data_.AmbientRateRtnMegaCps;
    }

    /** Return the effective SPAD count for the return signal. To obtain Real value it should be divided by 256 */
    uint16_t GetEffectiveSpadRtnCount()
    {
        return measurement_data_.EffectiveSpadRtnCount;
    }

    /** Denotes which zone and range scheduler stage the range data relates to. */
    uint8_t GetZoneId() { return measurement_data_.ZoneId; }

    /** Fractional part of range distance. Final value is a FixPoint168 value. */
    uint8_t GetRangeFractionalPart()
    {
        return measurement_data_.RangeFractionalPart;
    }

    /** Range Status for the current measurement. This is device dependent. Value = 0 means value is valid. */
    uint8_t GetRangeStatus() { return measurement_data_.RangeStatus; }

    /**  get a ranging measurement from the device
      @returns measurement data struct
    */
    VL53L0X_RangingMeasurementData_t getSingleRangingMeasurement();

    /**  Single shot ranging. Be sure to check the return of readRangeStatus to before using the return value!
        @return Distance in millimeters if valid
    */
    uint16_t readRange(void);

    /**  Request ranging success/error message (retrieve after ranging)
        @returns One of possible VL6180X_ERROR_* values
    */
    uint8_t readRangeStatus(void);

    /**  Start a range operation
        @return true if range operation successfully started.
    */
    bool startRange(void);

    /**  Checks to see if a range operation has completed
        @return true if range operation completed or an error has happened
    */
    bool isRangeComplete(void);

    /**  Wait until Range operation has completed.
        @return true if range operation completed, false if error.
    */
    bool waitRangeComplete(void);

    /**  Return the range in mm for the last operation.
        @return Range in mm.
    */
    uint16_t readRangeResult(void);

    /**  Start a continuous range operation
        @param period_ms inter measurement period in milliseconds
        @return True if successful, false otherwise
    */
    bool startRangeContinuous(uint16_t period_ms = 50);

    /**  Stop a continuous ranging operation */
    void stopRangeContinuous(void);

    /**  timeout status
      @returns True if timeout has occurred, False otherwise
    */
    bool timeoutOccurred(void) { return false; }

    bool configSensor(VL53L0X_Sense_config_t vl_config);

    /**  Wrapper to ST library code to budget how long a measurementshould take
        @param  budget_us the new budget
        @returns True if success
    */
    bool setMeasurementTimingBudgetMicroSeconds(uint32_t budget_us);

    /**  Wrapper to ST library code to budget how long a measurement should take
        @returns the current budget time in microseconds.
    */
    uint32_t getMeasurementTimingBudgetMicroSeconds(void);

    /** Sets the VCSEL pulse period.
        @param   VcselPeriodType VCSEL period identifier (pre-range|final).
        @param   VCSELPulsePeriod VCSEL period value
        @returns True if success
    */
    bool setVcselPulsePeriod(VL53L0X_VcselPeriod VcselPeriodType,
                             uint8_t             VCSELPulsePeriod);

    /** Gets the VCSEL pulse period.
        @param   VcselPeriodType VCSEL period identifier (pre-range|final).
        @returns the current pulse peried for the given type.
    */
    uint8_t getVcselPulsePeriod(VL53L0X_VcselPeriod VcselPeriodType);

    /**  Enable/Disable a specific limit check
        @param LimitCheckId Limit Check ID (0<= LimitCheckId < VL53L0X_GetNumberOfLimitCheck() ).
        @param LimitCheckEnable if 1 the check limit corresponding to LimitCheckId is Enabled
                if 0 the check limit corresponding to LimitCheckId is disabled
        @return  true if succeeded
    */
    bool setLimitCheckEnable(uint16_t LimitCheckId, uint8_t LimitCheckEnable);

    /**  Get specific limit check enable state
        @param LimitCheckId Limit Check ID (0<= LimitCheckId < VL53L0X_GetNumberOfLimitCheck() ).
        @return  current state of limit enabled
    */
    uint8_t getLimitCheckEnable(uint16_t LimitCheckId);

    /**  Set a specific limit check value
        @param  LimitCheckId Limit Check ID (0<= LimitCheckId < VL53L0X_GetNumberOfLimitCheck() ).
        @param  LimitCheckValue  Limit Check Value
        @return  true if succeeded.
    */
    bool setLimitCheckValue(uint16_t       LimitCheckId,
                            FixPoint1616_t LimitCheckValue);

    /**  Get a specific limit check value
        @param   LimitCheckId  Limit Check ID (0<= LimitCheckId < VL53L0X_GetNumberOfLimitCheck() ).
        @return  limit check value in FixPoint1616
    */
    FixPoint1616_t getLimitCheckValue(uint16_t LimitCheckId);

  private:
    Config    config_;
    I2CHandle i2c_;

    VL53L0X_RangingMeasurementData_t measurement_data_;

    VL53L0X_Error Status
        = VL53L0X_ERROR_NONE; ///< indicates whether or not the sensor has encountered an error

    Result InitI2C();

    VL53L0X_Dev_t        MyDevice;
    VL53L0X_Dev_t*       pMyDevice = &MyDevice;
    VL53L0X_DeviceInfo_t DeviceInfo;

    uint8_t _rangeStatus;
}; // class
/** @} */

} // namespace daisy
#endif
