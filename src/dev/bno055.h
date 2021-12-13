#pragma once
#ifndef DSY_BNO055_H
#define DSY_BNO055_H

/** BNO055 Address A **/
#define BNO055_ADDRESS_A (0x28)
/** BNO055 Address B **/
#define BNO055_ADDRESS_B (0x29)
/** BNO055 ID **/
#define BNO055_ID (0xA0)

/** Offsets registers **/
#define NUM_BNO055_OFFSET_REGISTERS (22)

#define SENSORS_DPS_TO_RADS (0.017453293F) /**< Degrees/s to rad/s multiplier */

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** I2C Transport for Bno055 */
class Bno055I2CTransport
{
  public:
    Bno055I2CTransport() {}
    ~Bno055I2CTransport() {}

    struct Config
    {
        I2CHandle::Config::Peripheral periph;
        I2CHandle::Config::Speed      speed;
        dsy_gpio_pin                  scl;
        dsy_gpio_pin                  sda;

        uint8_t address;

        Config()
        {
            address = BNO055_ADDRESS_A;

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

    void ReadLen(uint8_t reg, uint8_t *buff, uint16_t size)
    {
        Write(&reg, 1);
        Read(buff, size);
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

    /**  Reads an 8 bit value
        \param reg the register address to read from
        \returns the 16 bit data value read from the device
    */
    uint8_t Read8(uint8_t reg)
    {
        uint8_t buffer;
        ReadLen(reg, &buffer, 1);
        return buffer;
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

/** \brief Device support for BNO055 Humidity Pressure Sensor
    @author beserge
    @date December 2021
*/
template <typename Transport>
class Bno055
{
  public:
    Bno055() {}
    ~Bno055() {}

    struct Config
    {
        typename Transport::Config transport_config;
        mode_t                     init_mode;

        Config() { init_mode = OPERATION_MODE_NDOF; }
    };

    enum Result
    {
        OK = 0,
        ERR
    };

    struct BnoVector3
    {
        float x;
        float y;
        float z;
    };

    struct Quaternion
    {
        float w;
        float x;
        float y;
        float z;
    };

    /** A structure to represent offsets **/
    struct offsets_t
    {
        int16_t accel_offset_x; /**< x acceleration offset */
        int16_t accel_offset_y; /**< y acceleration offset */
        int16_t accel_offset_z; /**< z acceleration offset */

        int16_t mag_offset_x; /**< x magnetometer offset */
        int16_t mag_offset_y; /**< y magnetometer offset */
        int16_t mag_offset_z; /**< z magnetometer offset */

        int16_t gyro_offset_x; /**< x gyroscrope offset */
        int16_t gyro_offset_y; /**< y gyroscrope offset */
        int16_t gyro_offset_z; /**< z gyroscrope offset */

        int16_t accel_radius; /**< acceleration radius */

        int16_t mag_radius; /**< magnetometer radius */
    };


    /* Sensor event (36 bytes) */
    struct sensors_event_t
    {
        int32_t version;
        int32_t sensor_id;
        int32_t type;
        int32_t reserved0;
        int32_t timestamp;
        union
        {
            float      data[4];
            BnoVector3 acceleration;
            BnoVector3 magnetic;
            BnoVector3 orientation;
            BnoVector3 gyro;
            float      temperature;
            float      distance;
            float      light;
            float      pressure;
            float      relative_humidity;
            float      current;
            float      voltage;
            // sensors_color_t color;
        };
    };


    /** BNO055 Registers **/
    typedef enum
    {
        /* Page id register definition */
        BNO055_PAGE_ID_ADDR = 0X07,

        /* PAGE0 REGISTER DEFINITION START*/
        BNO055_CHIP_ID_ADDR       = 0x00,
        BNO055_ACCEL_REV_ID_ADDR  = 0x01,
        BNO055_MAG_REV_ID_ADDR    = 0x02,
        BNO055_GYRO_REV_ID_ADDR   = 0x03,
        BNO055_SW_REV_ID_LSB_ADDR = 0x04,
        BNO055_SW_REV_ID_MSB_ADDR = 0x05,
        BNO055_BL_REV_ID_ADDR     = 0X06,

        /* Accel data register */
        BNO055_ACCEL_DATA_X_LSB_ADDR = 0X08,
        BNO055_ACCEL_DATA_X_MSB_ADDR = 0X09,
        BNO055_ACCEL_DATA_Y_LSB_ADDR = 0X0A,
        BNO055_ACCEL_DATA_Y_MSB_ADDR = 0X0B,
        BNO055_ACCEL_DATA_Z_LSB_ADDR = 0X0C,
        BNO055_ACCEL_DATA_Z_MSB_ADDR = 0X0D,

        /* Mag data register */
        BNO055_MAG_DATA_X_LSB_ADDR = 0X0E,
        BNO055_MAG_DATA_X_MSB_ADDR = 0X0F,
        BNO055_MAG_DATA_Y_LSB_ADDR = 0X10,
        BNO055_MAG_DATA_Y_MSB_ADDR = 0X11,
        BNO055_MAG_DATA_Z_LSB_ADDR = 0X12,
        BNO055_MAG_DATA_Z_MSB_ADDR = 0X13,

        /* Gyro data registers */
        BNO055_GYRO_DATA_X_LSB_ADDR = 0X14,
        BNO055_GYRO_DATA_X_MSB_ADDR = 0X15,
        BNO055_GYRO_DATA_Y_LSB_ADDR = 0X16,
        BNO055_GYRO_DATA_Y_MSB_ADDR = 0X17,
        BNO055_GYRO_DATA_Z_LSB_ADDR = 0X18,
        BNO055_GYRO_DATA_Z_MSB_ADDR = 0X19,

        /* Euler data registers */
        BNO055_EULER_H_LSB_ADDR = 0X1A,
        BNO055_EULER_H_MSB_ADDR = 0X1B,
        BNO055_EULER_R_LSB_ADDR = 0X1C,
        BNO055_EULER_R_MSB_ADDR = 0X1D,
        BNO055_EULER_P_LSB_ADDR = 0X1E,
        BNO055_EULER_P_MSB_ADDR = 0X1F,

        /* Quaternion data registers */
        BNO055_QUATERNION_DATA_W_LSB_ADDR = 0X20,
        BNO055_QUATERNION_DATA_W_MSB_ADDR = 0X21,
        BNO055_QUATERNION_DATA_X_LSB_ADDR = 0X22,
        BNO055_QUATERNION_DATA_X_MSB_ADDR = 0X23,
        BNO055_QUATERNION_DATA_Y_LSB_ADDR = 0X24,
        BNO055_QUATERNION_DATA_Y_MSB_ADDR = 0X25,
        BNO055_QUATERNION_DATA_Z_LSB_ADDR = 0X26,
        BNO055_QUATERNION_DATA_Z_MSB_ADDR = 0X27,

        /* Linear acceleration data registers */
        BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR = 0X28,
        BNO055_LINEAR_ACCEL_DATA_X_MSB_ADDR = 0X29,
        BNO055_LINEAR_ACCEL_DATA_Y_LSB_ADDR = 0X2A,
        BNO055_LINEAR_ACCEL_DATA_Y_MSB_ADDR = 0X2B,
        BNO055_LINEAR_ACCEL_DATA_Z_LSB_ADDR = 0X2C,
        BNO055_LINEAR_ACCEL_DATA_Z_MSB_ADDR = 0X2D,

        /* Gravity data registers */
        BNO055_GRAVITY_DATA_X_LSB_ADDR = 0X2E,
        BNO055_GRAVITY_DATA_X_MSB_ADDR = 0X2F,
        BNO055_GRAVITY_DATA_Y_LSB_ADDR = 0X30,
        BNO055_GRAVITY_DATA_Y_MSB_ADDR = 0X31,
        BNO055_GRAVITY_DATA_Z_LSB_ADDR = 0X32,
        BNO055_GRAVITY_DATA_Z_MSB_ADDR = 0X33,

        /* Temperature data register */
        BNO055_TEMP_ADDR = 0X34,

        /* Status registers */
        BNO055_CALIB_STAT_ADDR      = 0X35,
        BNO055_SELFTEST_RESULT_ADDR = 0X36,
        BNO055_INTR_STAT_ADDR       = 0X37,

        BNO055_SYS_CLK_STAT_ADDR = 0X38,
        BNO055_SYS_STAT_ADDR     = 0X39,
        BNO055_SYS_ERR_ADDR      = 0X3A,

        /* Unit selection register */
        BNO055_UNIT_SEL_ADDR = 0X3B,

        /* Mode registers */
        BNO055_OPR_MODE_ADDR = 0X3D,
        BNO055_PWR_MODE_ADDR = 0X3E,

        BNO055_SYS_TRIGGER_ADDR = 0X3F,
        BNO055_TEMP_SOURCE_ADDR = 0X40,

        /* Axis remap registers */
        BNO055_AXIS_MAP_CONFIG_ADDR = 0X41,
        BNO055_AXIS_MAP_SIGN_ADDR   = 0X42,

        /* SIC registers */
        BNO055_SIC_MATRIX_0_LSB_ADDR = 0X43,
        BNO055_SIC_MATRIX_0_MSB_ADDR = 0X44,
        BNO055_SIC_MATRIX_1_LSB_ADDR = 0X45,
        BNO055_SIC_MATRIX_1_MSB_ADDR = 0X46,
        BNO055_SIC_MATRIX_2_LSB_ADDR = 0X47,
        BNO055_SIC_MATRIX_2_MSB_ADDR = 0X48,
        BNO055_SIC_MATRIX_3_LSB_ADDR = 0X49,
        BNO055_SIC_MATRIX_3_MSB_ADDR = 0X4A,
        BNO055_SIC_MATRIX_4_LSB_ADDR = 0X4B,
        BNO055_SIC_MATRIX_4_MSB_ADDR = 0X4C,
        BNO055_SIC_MATRIX_5_LSB_ADDR = 0X4D,
        BNO055_SIC_MATRIX_5_MSB_ADDR = 0X4E,
        BNO055_SIC_MATRIX_6_LSB_ADDR = 0X4F,
        BNO055_SIC_MATRIX_6_MSB_ADDR = 0X50,
        BNO055_SIC_MATRIX_7_LSB_ADDR = 0X51,
        BNO055_SIC_MATRIX_7_MSB_ADDR = 0X52,
        BNO055_SIC_MATRIX_8_LSB_ADDR = 0X53,
        BNO055_SIC_MATRIX_8_MSB_ADDR = 0X54,

        /* Accelerometer Offset registers */
        ACCEL_OFFSET_X_LSB_ADDR = 0X55,
        ACCEL_OFFSET_X_MSB_ADDR = 0X56,
        ACCEL_OFFSET_Y_LSB_ADDR = 0X57,
        ACCEL_OFFSET_Y_MSB_ADDR = 0X58,
        ACCEL_OFFSET_Z_LSB_ADDR = 0X59,
        ACCEL_OFFSET_Z_MSB_ADDR = 0X5A,

        /* Magnetometer Offset registers */
        MAG_OFFSET_X_LSB_ADDR = 0X5B,
        MAG_OFFSET_X_MSB_ADDR = 0X5C,
        MAG_OFFSET_Y_LSB_ADDR = 0X5D,
        MAG_OFFSET_Y_MSB_ADDR = 0X5E,
        MAG_OFFSET_Z_LSB_ADDR = 0X5F,
        MAG_OFFSET_Z_MSB_ADDR = 0X60,

        /* Gyroscope Offset register s*/
        GYRO_OFFSET_X_LSB_ADDR = 0X61,
        GYRO_OFFSET_X_MSB_ADDR = 0X62,
        GYRO_OFFSET_Y_LSB_ADDR = 0X63,
        GYRO_OFFSET_Y_MSB_ADDR = 0X64,
        GYRO_OFFSET_Z_LSB_ADDR = 0X65,
        GYRO_OFFSET_Z_MSB_ADDR = 0X66,

        /* Radius registers */
        ACCEL_RADIUS_LSB_ADDR = 0X67,
        ACCEL_RADIUS_MSB_ADDR = 0X68,
        MAG_RADIUS_LSB_ADDR   = 0X69,
        MAG_RADIUS_MSB_ADDR   = 0X6A
    } reg_t;

    /** Sensor types */
    typedef enum
    {
        SENSOR_TYPE_ACCELEROMETER  = (1), /**< Gravity + linear acceleration */
        SENSOR_TYPE_MAGNETIC_FIELD = (2),
        SENSOR_TYPE_ORIENTATION    = (3),
        SENSOR_TYPE_GYROSCOPE      = (4),
        SENSOR_TYPE_LIGHT          = (5),
        SENSOR_TYPE_PRESSURE       = (6),
        SENSOR_TYPE_PROXIMITY      = (8),
        SENSOR_TYPE_GRAVITY        = (9),
        SENSOR_TYPE_LINEAR_ACCELERATION = (10), /**< Acceleration w/o gravity */
        SENSOR_TYPE_ROTATION_VECTOR     = (11),
        SENSOR_TYPE_RELATIVE_HUMIDITY   = (12),
        SENSOR_TYPE_AMBIENT_TEMPERATURE = (13),
        SENSOR_TYPE_VOLTAGE             = (15),
        SENSOR_TYPE_CURRENT             = (16),
        SENSOR_TYPE_COLOR               = (17)
    } sensors_type_t;

    /** BNO055 power settings */
    typedef enum
    {
        POWER_MODE_NORMAL   = 0X00,
        POWER_MODE_LOWPOWER = 0X01,
        POWER_MODE_SUSPEND  = 0X02
    } powermode_t;

    /** Operation mode settings **/
    typedef enum
    {
        OPERATION_MODE_CONFIG       = 0X00,
        OPERATION_MODE_ACCONLY      = 0X01,
        OPERATION_MODE_MAGONLY      = 0X02,
        OPERATION_MODE_GYRONLY      = 0X03,
        OPERATION_MODE_ACCMAG       = 0X04,
        OPERATION_MODE_ACCGYRO      = 0X05,
        OPERATION_MODE_MAGGYRO      = 0X06,
        OPERATION_MODE_AMG          = 0X07,
        OPERATION_MODE_IMUPLUS      = 0X08,
        OPERATION_MODE_COMPASS      = 0X09,
        OPERATION_MODE_M4G          = 0X0A,
        OPERATION_MODE_NDOF_FMC_OFF = 0X0B,
        OPERATION_MODE_NDOF         = 0X0C
    } opmode_t;
    opmode_t _mode;

    /** Remap settings **/
    typedef enum
    {
        REMAP_CONFIG_P0 = 0x21,
        REMAP_CONFIG_P1 = 0x24, // default
        REMAP_CONFIG_P2 = 0x24,
        REMAP_CONFIG_P3 = 0x21,
        REMAP_CONFIG_P4 = 0x24,
        REMAP_CONFIG_P5 = 0x21,
        REMAP_CONFIG_P6 = 0x21,
        REMAP_CONFIG_P7 = 0x24
    } axis_remap_config_t;

    /** Remap Signs **/
    typedef enum
    {
        REMAP_SIGN_P0 = 0x04,
        REMAP_SIGN_P1 = 0x00, // default
        REMAP_SIGN_P2 = 0x06,
        REMAP_SIGN_P3 = 0x02,
        REMAP_SIGN_P4 = 0x03,
        REMAP_SIGN_P5 = 0x01,
        REMAP_SIGN_P6 = 0x07,
        REMAP_SIGN_P7 = 0x05
    } axis_remap_sign_t;

    /** A structure to represent revisions **/
    typedef struct
    {
        uint8_t  accel_rev; /**< acceleration rev */
        uint8_t  mag_rev;   /**< magnetometer rev */
        uint8_t  gyro_rev;  /**< gyroscrope rev */
        uint16_t sw_rev;    /**< SW rev */
        uint8_t  bl_rev;    /**< bootloader rev */
    } rev_info_t;

    /** Vector Mappings **/
    typedef enum
    {
        VECTOR_ACCELEROMETER = BNO055_ACCEL_DATA_X_LSB_ADDR,
        VECTOR_MAGNETOMETER  = BNO055_MAG_DATA_X_LSB_ADDR,
        VECTOR_GYROSCOPE     = BNO055_GYRO_DATA_X_LSB_ADDR,
        VECTOR_EULER         = BNO055_EULER_H_LSB_ADDR,
        VECTOR_LINEARACCEL   = BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR,
        VECTOR_GRAVITY       = BNO055_GRAVITY_DATA_X_LSB_ADDR
    } vector_type_t;

    /** Initialize the Bno055 device
        \param config Configuration settings
    */
    Result Init(Config config)
    {
        config_ = config;

        transport_.Init(config_.transport_config);

        /* Make sure we have the right device */
        uint8_t id = Read8(BNO055_CHIP_ID_ADDR);
        if(id != BNO055_ID)
        {
            System::Delay(1000); // hold on for boot
            id = Read8(BNO055_CHIP_ID_ADDR);
            if(id != BNO055_ID)
            {
                return ERR; // still not? ok bail
            }
        }

        /* Switch to config mode (just in case since this is the default) */
        SetMode(OPERATION_MODE_CONFIG);

        /* Reset */
        Write8(BNO055_SYS_TRIGGER_ADDR, 0x20);
        /* Delay incrased to 30ms due to power issues https://tinyurl.com/y375z699 */
        System::Delay(30);
        while(Read8(BNO055_CHIP_ID_ADDR) != BNO055_ID)
        {
            System::Delay(10);
        }
        System::Delay(50);

        /* Set to normal power mode */
        Write8(BNO055_PWR_MODE_ADDR, POWER_MODE_NORMAL);
        System::Delay(10);

        Write8(BNO055_PAGE_ID_ADDR, 0);

        Write8(BNO055_SYS_TRIGGER_ADDR, 0x0);
        System::Delay(10);
        /* Set the requested operating mode (see section 3.3) */
        SetMode(config_.init_mode);
        System::Delay(20);

        return GetTransportError();
    }

    /**  Writes an 8 bit value
        \param reg the register address to write to
        \param value the value to write to the register
    */
    void Write8(uint8_t reg, uint8_t value)
    {
        return transport_.Write8(reg, value);
    }

    /**  Reads an 8 bit value
        \param reg the register address to read from
        \returns the data uint8_t read from the device
    */
    uint8_t Read8(uint8_t reg) { return transport_.Read8(reg); }

    void ReadLen(uint8_t reg, uint8_t *buff, uint8_t len)
    {
        transport_.ReadLen(reg, buff, len);
    }


    /** Get and reset the transport error flag
        \return Whether the transport has errored since the last check
    */
    Result GetTransportError() { return transport_.GetError() ? ERR : OK; }

    /**  Puts the chip in the specified operating mode
        \param  mode mode values
            [OPERATION_MODE_CONFIG,
            OPERATION_MODE_ACCONLY,
            OPERATION_MODE_MAGONLY,
            OPERATION_MODE_GYRONLY,
            OPERATION_MODE_ACCMAG,
            OPERATION_MODE_ACCGYRO,
            OPERATION_MODE_MAGGYRO,
            OPERATION_MODE_AMG,
            OPERATION_MODE_IMUPLUS,
            OPERATION_MODE_COMPASS,
            OPERATION_MODE_M4G,
            OPERATION_MODE_NDOF_FMC_OFF,
            OPERATION_MODE_NDOF]
    */
    void SetMode(opmode_t mode)
    {
        _mode = mode;
        Write8(BNO055_OPR_MODE_ADDR, _mode);
        System::Delay(30);
    }

    /**  Changes the chip's axis remap
        \param  remapcode remap code possible values
            [REMAP_CONFIG_P0
            REMAP_CONFIG_P1 (default)
            REMAP_CONFIG_P2
            REMAP_CONFIG_P3
            REMAP_CONFIG_P4
            REMAP_CONFIG_P5
            REMAP_CONFIG_P6
            REMAP_CONFIG_P7]
    */
    void SetAxisRemap(axis_remap_config_t remapcode)
    {
        opmode_t modeback = _mode;

        SetMode(OPERATION_MODE_CONFIG);
        System::Delay(25);
        Write8(BNO055_AXIS_MAP_CONFIG_ADDR, remapcode);
        System::Delay(10);
        /* Set the requested operating mode (see section 3.3) */
        SetMode(modeback);
        System::Delay(20);
    }

    /**  Changes the chip's axis signs
        \param  remapsign remap sign possible values
        [REMAP_SIGN_P0
        REMAP_SIGN_P1 (default)
        REMAP_SIGN_P2
        REMAP_SIGN_P3
        REMAP_SIGN_P4
        REMAP_SIGN_P5
        REMAP_SIGN_P6
        REMAP_SIGN_P7]
        */
    void SetAxisSign(axis_remap_sign_t remapsign)
    {
        opmode_t modeback = _mode;

        SetMode(OPERATION_MODE_CONFIG);
        System::Delay(25);
        Write8(BNO055_AXIS_MAP_SIGN_ADDR, remapsign);
        System::Delay(10);
        /* Set the requested operating mode (see section 3.3) */
        SetMode(modeback);
        System::Delay(20);
    }

    /**  Use the external 32.768KHz crystal
        \param  usextal use external crystal boolean
    */
    void SetExtCrystalUse(bool usextal)
    {
        opmode_t modeback = _mode;

        /* Switch to config mode (just in case since this is the default) */
        SetMode(OPERATION_MODE_CONFIG);
        System::Delay(25);
        Write8(BNO055_PAGE_ID_ADDR, 0);
        if(usextal)
        {
            Write8(BNO055_SYS_TRIGGER_ADDR, 0x80);
        }
        else
        {
            Write8(BNO055_SYS_TRIGGER_ADDR, 0x00);
        }
        System::Delay(10);
        /* Set the requested operating mode (see section 3.3) */
        SetMode(modeback);
        System::Delay(20);
    }

    /** Gets the latest system status info
        \param  system_status system status info
        \param  self_test_result self test result
        \param  system_error system error info
    */
    void GetSystemStatus(uint8_t *system_status,
                         uint8_t *self_test_result,
                         uint8_t *system_error)
    {
        Write8(BNO055_PAGE_ID_ADDR, 0);

        /* System Status (see section 4.3.58)
            0 = Idle
            1 = System Error
            2 = Initializing Peripherals
            3 = System Iniitalization
            4 = Executing Self-Test
            5 = Sensor fusio algorithm running
            6 = System running without fusion algorithms
        */

        if(system_status != 0)
            *system_status = Read8(BNO055_SYS_STAT_ADDR);

        /* Self Test Results
            1 = test passed, 0 = test failed
            Bit 0 = Accelerometer self test
            Bit 1 = Magnetometer self test
            Bit 2 = Gyroscope self test
            Bit 3 = MCU self test
            0x0F = all good!
        */

        if(self_test_result != 0)
            *self_test_result = Read8(BNO055_SELFTEST_RESULT_ADDR);

        /* System Error (see section 4.3.59)
            0 = No error
            1 = Peripheral initialization error
            2 = System initialization error
            3 = Self test result failed
            4 = Register map value out of range
            5 = Register map address out of range
            6 = Register map write error
            7 = BNO low power mode not available for selected operat ion mode
            8 = Accelerometer power mode not available
            9 = Fusion algorithm configuration error
            A = Sensor configuration error
        */

        if(system_error != 0)
            *system_error = Read8(BNO055_SYS_ERR_ADDR);

        System::Delay(200);
    }

    /**  Gets the chip revision numbers
        \param  info revision info
    */
    void GetRevInfo(rev_info_t *info)
    {
        uint8_t a, b;

        memset(info, 0, sizeof(rev_info_t));

        /* Check the accelerometer revision */
        info->accel_rev = Read8(BNO055_ACCEL_REV_ID_ADDR);

        /* Check the magnetometer revision */
        info->mag_rev = Read8(BNO055_MAG_REV_ID_ADDR);

        /* Check the gyroscope revision */
        info->gyro_rev = Read8(BNO055_GYRO_REV_ID_ADDR);

        /* Check the SW revision */
        info->bl_rev = Read8(BNO055_BL_REV_ID_ADDR);

        a            = Read8(BNO055_SW_REV_ID_LSB_ADDR);
        b            = Read8(BNO055_SW_REV_ID_MSB_ADDR);
        info->sw_rev = (((uint16_t)b) << 8) | ((uint16_t)a);
    }

    /**  Gets current calibration state.  Each value should be a uint8_t
              pointer and it will be set to 0 if not calibrated and 3 if
              fully calibrated.
              See section 34.3.54
      \param  sys Current system calibration status, depends on status of all sensors, read-only
      \param  gyro Current calibration status of Gyroscope, read-only
      \param  accel Current calibration status of Accelerometer, read-only
      \param  mag Current calibration status of Magnetometer, read-only
    */
    void
    GetCalibration(uint8_t *sys, uint8_t *gyro, uint8_t *accel, uint8_t *mag)
    {
        uint8_t calData = Read8(BNO055_CALIB_STAT_ADDR);
        if(sys != NULL)
        {
            *sys = (calData >> 6) & 0x03;
        }
        if(gyro != NULL)
        {
            *gyro = (calData >> 4) & 0x03;
        }
        if(accel != NULL)
        {
            *accel = (calData >> 2) & 0x03;
        }
        if(mag != NULL)
        {
            *mag = calData & 0x03;
        }
    }

    /**  Gets the temperature in degrees celsius
        \return temperature in degrees celsius
    */
    int8_t GetTemp()
    {
        int8_t temp = (int8_t)(Read8(BNO055_TEMP_ADDR));
        return temp;
    }

    /**   Gets a vector reading from the specified source
        \param   vector_type possible vector type values
                 [VECTOR_ACCELEROMETER
                  VECTOR_MAGNETOMETER
                  VECTOR_GYROSCOPE
                  VECTOR_EULER
                  VECTOR_LINEARACCEL
                  VECTOR_GRAVITY]
        \return  vector from specified source
    */
    BnoVector3 GetVector(vector_type_t vector_type)
    {
        BnoVector3 xyz;
        uint8_t    buffer[6] = {0, 0, 0, 0, 0, 0};

        int16_t x, y, z;
        x = y = z = 0;

        /* Read vector data (6 bytes) */
        ReadLen((reg_t)vector_type, buffer, 6);

        x = ((int16_t)buffer[0]) | (((int16_t)buffer[1]) << 8);
        y = ((int16_t)buffer[2]) | (((int16_t)buffer[3]) << 8);
        z = ((int16_t)buffer[4]) | (((int16_t)buffer[5]) << 8);

        /**
            Convert the value to an appropriate range (section 3.6.4)
            and assign the value to the Vector type
        */
        switch(vector_type)
        {
            case VECTOR_MAGNETOMETER:
                /* 1uT = 16 LSB */
                xyz.x = ((float)x) / 16.f;
                xyz.y = ((float)y) / 16.f;
                xyz.z = ((float)z) / 16.f;
                break;
            case VECTOR_GYROSCOPE:
                /* 1dps = 16 LSB */
                xyz.x = ((float)x) / 16.f;
                xyz.y = ((float)y) / 16.f;
                xyz.z = ((float)z) / 16.f;
                break;
            case VECTOR_EULER:
                /* 1 degree = 16 LSB */
                xyz.x = ((float)x) / 16.f;
                xyz.y = ((float)y) / 16.f;
                xyz.z = ((float)z) / 16.f;
                break;
            case VECTOR_ACCELEROMETER:
                /* 1m/s^2 = 100 LSB */
                xyz.x = ((float)x) / 100.f;
                xyz.y = ((float)y) / 100.f;
                xyz.z = ((float)z) / 100.f;
                break;
            case VECTOR_LINEARACCEL:
                /* 1m/s^2 = 100 LSB */
                xyz.x = ((float)x) / 100.f;
                xyz.y = ((float)y) / 100.f;
                xyz.z = ((float)z) / 100.f;
                break;
            case VECTOR_GRAVITY:
                /* 1m/s^2 = 100 LSB */
                xyz.x = ((float)x) / 100.f;
                xyz.y = ((float)y) / 100.f;
                xyz.z = ((float)z) / 100.f;
                break;
        }

        return xyz;
    }

    /**  Gets a quaternion reading from the specified source
        \return quaternion reading
    */
    Quaternion GetQuat()
    {
        uint8_t buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        Quaternion quat;

        int16_t x, y, z, w;
        x = y = z = w = 0;

        /* Read quat data (8 bytes) */
        ReadLen(BNO055_QUATERNION_DATA_W_LSB_ADDR, buffer, 8);
        w = (((uint16_t)buffer[1]) << 8) | ((uint16_t)buffer[0]);
        x = (((uint16_t)buffer[3]) << 8) | ((uint16_t)buffer[2]);
        y = (((uint16_t)buffer[5]) << 8) | ((uint16_t)buffer[4]);
        z = (((uint16_t)buffer[7]) << 8) | ((uint16_t)buffer[6]);

        /** Assign to Quaternion
            See https://cdn-shop.adafruit.com/datasheets/BST_BNO055_DS000_12.pdf
            3.6.5.5 Orientation (Quaternion)
        */
        const float scale = (1.0 / (1 << 14));
        quat.w            = scale * w;
        quat.x            = scale * x;
        quat.y            = scale * y;
        quat.z            = scale * z;
    }

    /**  Reads the sensor and returns the data as a sensors_event_t
        \param  event Event description
        \return always returns true
    */
    bool GetEvent(sensors_event_t *event)
    {
        /* Clear the event */
        memset(event, 0, sizeof(sensors_event_t));

        event->version   = sizeof(sensors_event_t);
        event->sensor_id = _sensorID;
        event->type      = SENSOR_TYPE_ORIENTATION;
        event->timestamp = System::GetNow();

        /* Get a Euler angle sample for orientation */
        BnoVector3 euler     = GetVector(VECTOR_EULER);
        event->orientation.x = euler.x;
        event->orientation.y = euler.y;
        event->orientation.z = euler.z;

        return true;
    }

    /**  Reads the sensor and returns the data as a sensors_event_t
        \param  event Event description
        \param  vec_type specify the type of reading
        \return always returns true
    */
    bool GetEvent(sensors_event_t *event, vector_type_t vec_type)
    {
        /* Clear the event */
        memset(event, 0, sizeof(sensors_event_t));

        event->version   = sizeof(sensors_event_t);
        event->sensor_id = _sensorID;
        event->timestamp = System::GetNow();

        // read the data according to vec_type
        BnoVector3 vec;
        if(vec_type == VECTOR_LINEARACCEL)
        {
            event->type = SENSOR_TYPE_LINEAR_ACCELERATION;
            vec         = GetVector(VECTOR_LINEARACCEL);

            event->acceleration.x = vec.x;
            event->acceleration.y = vec.y;
            event->acceleration.z = vec.z;
        }
        else if(vec_type == VECTOR_ACCELEROMETER)
        {
            event->type = SENSOR_TYPE_ACCELEROMETER;
            vec         = GetVector(VECTOR_ACCELEROMETER);

            event->acceleration.x = vec.x;
            event->acceleration.y = vec.y;
            event->acceleration.z = vec.z;
        }
        else if(vec_type == VECTOR_GRAVITY)
        {
            event->type = SENSOR_TYPE_GRAVITY;
            vec         = GetVector(VECTOR_GRAVITY);

            event->acceleration.x = vec.x;
            event->acceleration.y = vec.y;
            event->acceleration.z = vec.z;
        }
        else if(vec_type == VECTOR_EULER)
        {
            event->type = SENSOR_TYPE_ORIENTATION;
            vec         = GetVector(VECTOR_EULER);

            event->orientation.x = vec.x;
            event->orientation.y = vec.y;
            event->orientation.z = vec.z;
        }
        else if(vec_type == VECTOR_GYROSCOPE)
        {
            event->type = SENSOR_TYPE_GYROSCOPE;
            vec         = GetVector(VECTOR_GYROSCOPE);

            event->gyro.x = vec.x * SENSORS_DPS_TO_RADS;
            event->gyro.y = vec.y * SENSORS_DPS_TO_RADS;
            event->gyro.z = vec.z * SENSORS_DPS_TO_RADS;
        }
        else if(vec_type == VECTOR_MAGNETOMETER)
        {
            event->type = SENSOR_TYPE_MAGNETIC_FIELD;
            vec         = GetVector(VECTOR_MAGNETOMETER);

            event->magnetic.x = vec.x;
            event->magnetic.y = vec.y;
            event->magnetic.z = vec.z;
        }

        return true;
    }

    /**  Reads the sensor's offset registers into a byte array
        \param  calibData Calibration offset (buffer size should be 22)
        \return true if read is successful
    */
    bool GetSensorOffsets(uint8_t *calibData)
    {
        if(isFullyCalibrated())
        {
            opmode_t lastMode = _mode;
            SetMode(OPERATION_MODE_CONFIG);

            ReadLen(ACCEL_OFFSET_X_LSB_ADDR,
                    calibData,
                    NUM_BNO055_OFFSET_REGISTERS);

            SetMode(lastMode);
            return true;
        }
        return false;
    }

    /**  Reads the sensor's offset registers into an offset struct
        \param  offsets_type type of offsets
        \return true if read is successful
    */
    bool GetSensorOffsets(offsets_t &offsets_type)
    {
        if(isFullyCalibrated())
        {
            opmode_t lastMode = _mode;
            SetMode(OPERATION_MODE_CONFIG);
            System::Delay(25);

            /* Accel offset range depends on the G-range:
            +/-2g  = +/- 2000 mg
            +/-4g  = +/- 4000 mg
            +/-8g  = +/- 8000 mg
            +/-1§g = +/- 16000 mg */
            offsets_type.accel_offset_x = (Read8(ACCEL_OFFSET_X_MSB_ADDR) << 8)
                                          | (Read8(ACCEL_OFFSET_X_LSB_ADDR));
            offsets_type.accel_offset_y = (Read8(ACCEL_OFFSET_Y_MSB_ADDR) << 8)
                                          | (Read8(ACCEL_OFFSET_Y_LSB_ADDR));
            offsets_type.accel_offset_z = (Read8(ACCEL_OFFSET_Z_MSB_ADDR) << 8)
                                          | (Read8(ACCEL_OFFSET_Z_LSB_ADDR));

            /* Magnetometer offset range = +/- 6400 LSB where 1uT = 16 LSB */
            offsets_type.mag_offset_x = (Read8(MAG_OFFSET_X_MSB_ADDR) << 8)
                                        | (Read8(MAG_OFFSET_X_LSB_ADDR));
            offsets_type.mag_offset_y = (Read8(MAG_OFFSET_Y_MSB_ADDR) << 8)
                                        | (Read8(MAG_OFFSET_Y_LSB_ADDR));
            offsets_type.mag_offset_z = (Read8(MAG_OFFSET_Z_MSB_ADDR) << 8)
                                        | (Read8(MAG_OFFSET_Z_LSB_ADDR));

            /* Gyro offset range depends on the DPS range:
            2000 dps = +/- 32000 LSB
            1000 dps = +/- 16000 LSB
            500 dps = +/- 8000 LSB
            250 dps = +/- 4000 LSB
            125 dps = +/- 2000 LSB
            ... where 1 DPS = 16 LSB */
            offsets_type.gyro_offset_x = (Read8(GYRO_OFFSET_X_MSB_ADDR) << 8)
                                         | (Read8(GYRO_OFFSET_X_LSB_ADDR));
            offsets_type.gyro_offset_y = (Read8(GYRO_OFFSET_Y_MSB_ADDR) << 8)
                                         | (Read8(GYRO_OFFSET_Y_LSB_ADDR));
            offsets_type.gyro_offset_z = (Read8(GYRO_OFFSET_Z_MSB_ADDR) << 8)
                                         | (Read8(GYRO_OFFSET_Z_LSB_ADDR));

            /* Accelerometer radius = +/- 1000 LSB */
            offsets_type.accel_radius = (Read8(ACCEL_RADIUS_MSB_ADDR) << 8)
                                        | (Read8(ACCEL_RADIUS_LSB_ADDR));

            /* Magnetometer radius = +/- 960 LSB */
            offsets_type.mag_radius = (Read8(MAG_RADIUS_MSB_ADDR) << 8)
                                      | (Read8(MAG_RADIUS_LSB_ADDR));

            SetMode(lastMode);
            return true;
        }
        return false;
    }

    /**  Writes an array of calibration values to the sensor's offset
        \param  calibData
                calibration data
    */
    void SetSensorOffsets(const uint8_t *calibData)
    {
        opmode_t lastMode = _mode;
        SetMode(OPERATION_MODE_CONFIG);
        System::Delay(25);

        /* Note: Configuration will take place only when user writes to the last
         byte of each config data pair (ex. ACCEL_OFFSET_Z_MSB_ADDR, etc.).
         Therefore the last byte must be written whenever the user wants to
         changes the configuration. */

        /* A writeLen() would make this much cleaner */
        Write8(ACCEL_OFFSET_X_LSB_ADDR, calibData[0]);
        Write8(ACCEL_OFFSET_X_MSB_ADDR, calibData[1]);
        Write8(ACCEL_OFFSET_Y_LSB_ADDR, calibData[2]);
        Write8(ACCEL_OFFSET_Y_MSB_ADDR, calibData[3]);
        Write8(ACCEL_OFFSET_Z_LSB_ADDR, calibData[4]);
        Write8(ACCEL_OFFSET_Z_MSB_ADDR, calibData[5]);

        Write8(MAG_OFFSET_X_LSB_ADDR, calibData[6]);
        Write8(MAG_OFFSET_X_MSB_ADDR, calibData[7]);
        Write8(MAG_OFFSET_Y_LSB_ADDR, calibData[8]);
        Write8(MAG_OFFSET_Y_MSB_ADDR, calibData[9]);
        Write8(MAG_OFFSET_Z_LSB_ADDR, calibData[10]);
        Write8(MAG_OFFSET_Z_MSB_ADDR, calibData[11]);

        Write8(GYRO_OFFSET_X_LSB_ADDR, calibData[12]);
        Write8(GYRO_OFFSET_X_MSB_ADDR, calibData[13]);
        Write8(GYRO_OFFSET_Y_LSB_ADDR, calibData[14]);
        Write8(GYRO_OFFSET_Y_MSB_ADDR, calibData[15]);
        Write8(GYRO_OFFSET_Z_LSB_ADDR, calibData[16]);
        Write8(GYRO_OFFSET_Z_MSB_ADDR, calibData[17]);

        Write8(ACCEL_RADIUS_LSB_ADDR, calibData[18]);
        Write8(ACCEL_RADIUS_MSB_ADDR, calibData[19]);

        Write8(MAG_RADIUS_LSB_ADDR, calibData[20]);
        Write8(MAG_RADIUS_MSB_ADDR, calibData[21]);

        setMode(lastMode);
    }

    /**  Writes to the sensor's offset registers from an offset struct
        \param  offsets_type
            accel_offset_x = acceleration offset x
            accel_offset_y = acceleration offset y
            accel_offset_z = acceleration offset z

            mag_offset_x   = magnetometer offset x
            mag_offset_y   = magnetometer offset y
            mag_offset_z   = magnetometer offset z

            gyro_offset_x  = gyroscrope offset x
            gyro_offset_y  = gyroscrope offset y
            gyro_offset_z  = gyroscrope offset z
    */
    void SetSensorOffsets(const offsets_t &offsets_type)
    {
        opmode_t lastMode = _mode;
        SetMode(OPERATION_MODE_CONFIG);
        System::Delay(25);

        /* Note: Configuration will take place only when user writes to the last
         byte of each config data pair (ex. ACCEL_OFFSET_Z_MSB_ADDR, etc.).
         Therefore the last byte must be written whenever the user wants to
         changes the configuration. */

        Write8(ACCEL_OFFSET_X_LSB_ADDR, (offsets_type.accel_offset_x) & 0x0FF);
        Write8(ACCEL_OFFSET_X_MSB_ADDR,
               (offsets_type.accel_offset_x >> 8) & 0x0FF);
        Write8(ACCEL_OFFSET_Y_LSB_ADDR, (offsets_type.accel_offset_y) & 0x0FF);
        Write8(ACCEL_OFFSET_Y_MSB_ADDR,
               (offsets_type.accel_offset_y >> 8) & 0x0FF);
        Write8(ACCEL_OFFSET_Z_LSB_ADDR, (offsets_type.accel_offset_z) & 0x0FF);
        Write8(ACCEL_OFFSET_Z_MSB_ADDR,
               (offsets_type.accel_offset_z >> 8) & 0x0FF);

        Write8(MAG_OFFSET_X_LSB_ADDR, (offsets_type.mag_offset_x) & 0x0FF);
        Write8(MAG_OFFSET_X_MSB_ADDR, (offsets_type.mag_offset_x >> 8) & 0x0FF);
        Write8(MAG_OFFSET_Y_LSB_ADDR, (offsets_type.mag_offset_y) & 0x0FF);
        Write8(MAG_OFFSET_Y_MSB_ADDR, (offsets_type.mag_offset_y >> 8) & 0x0FF);
        Write8(MAG_OFFSET_Z_LSB_ADDR, (offsets_type.mag_offset_z) & 0x0FF);
        Write8(MAG_OFFSET_Z_MSB_ADDR, (offsets_type.mag_offset_z >> 8) & 0x0FF);

        Write8(GYRO_OFFSET_X_LSB_ADDR, (offsets_type.gyro_offset_x) & 0x0FF);
        Write8(GYRO_OFFSET_X_MSB_ADDR,
               (offsets_type.gyro_offset_x >> 8) & 0x0FF);
        Write8(GYRO_OFFSET_Y_LSB_ADDR, (offsets_type.gyro_offset_y) & 0x0FF);
        Write8(GYRO_OFFSET_Y_MSB_ADDR,
               (offsets_type.gyro_offset_y >> 8) & 0x0FF);
        Write8(GYRO_OFFSET_Z_LSB_ADDR, (offsets_type.gyro_offset_z) & 0x0FF);
        Write8(GYRO_OFFSET_Z_MSB_ADDR,
               (offsets_type.gyro_offset_z >> 8) & 0x0FF);

        Write8(ACCEL_RADIUS_LSB_ADDR, (offsets_type.accel_radius) & 0x0FF);
        Write8(ACCEL_RADIUS_MSB_ADDR, (offsets_type.accel_radius >> 8) & 0x0FF);

        Write8(MAG_RADIUS_LSB_ADDR, (offsets_type.mag_radius) & 0x0FF);
        Write8(MAG_RADIUS_MSB_ADDR, (offsets_type.mag_radius >> 8) & 0x0FF);

        SetMode(lastMode);
    }

    /**  Checks of all cal status values are set to 3 (fully calibrated)
        \return status of calibration
    */
    bool isFullyCalibrated()
    {
        uint8_t system, gyro, accel, mag;
        GetCalibration(&system, &gyro, &accel, &mag);

        switch(_mode)
        {
            case OPERATION_MODE_ACCONLY: return (accel == 3);
            case OPERATION_MODE_MAGONLY: return (mag == 3);
            case OPERATION_MODE_GYRONLY:
            case OPERATION_MODE_M4G: /* No magnetometer calibration required. */
                return (gyro == 3);
            case OPERATION_MODE_ACCMAG:
            case OPERATION_MODE_COMPASS: return (accel == 3 && mag == 3);
            case OPERATION_MODE_ACCGYRO:
            case OPERATION_MODE_IMUPLUS: return (accel == 3 && gyro == 3);
            case OPERATION_MODE_MAGGYRO: return (mag == 3 && gyro == 3);
            default:
                return (system == 3 && gyro == 3 && accel == 3 && mag == 3);
        }
    }

    /**  Enter Suspend mode (i.e., sleep) */
    void EnterSuspendMode()
    {
        opmode_t modeback = _mode;

        /* Switch to config mode (just in case since this is the default) */
        SetMode(OPERATION_MODE_CONFIG);
        System::Delay(25);
        Write8(BNO055_PWR_MODE_ADDR, 0x02);
        /* Set the requested operating mode (see section 3.3) */
        SetMode(modeback);
        System::Delay(20);
    }

    /**  Enter Normal mode (i.e., wake) */
    void EnterNormalMode()
    {
        opmode_t modeback = _mode;

        /* Switch to config mode (just in case since this is the default) */
        SetMode(OPERATION_MODE_CONFIG);
        System::Delay(25);
        Write8(BNO055_PWR_MODE_ADDR, 0x00);
        /* Set the requested operating mode (see section 3.3) */
        SetMode(modeback);
        System::Delay(20);
    }

  private:
    int32_t   _sensorID;
    Config    config_;
    Transport transport_;
}; // namespace daisy

/** @} */

using Bno055I2C = Bno055<Bno055I2CTransport>;
} // namespace daisy
#endif