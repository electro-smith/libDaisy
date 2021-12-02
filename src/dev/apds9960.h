#pragma once
#ifndef DSY_APDS9960_H
#define DSY_APDS9960_H

#define APDS9960_ADDRESS (0x39) /**< I2C Address */

#define APDS9960_UP 0x01    /**< Gesture Up */
#define APDS9960_DOWN 0x02  /**< Gesture Down */
#define APDS9960_LEFT 0x03  /**< Gesture Left */
#define APDS9960_RIGHT 0x04 /**< Gesture Right */

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** I2C Transport for APDS9960 */
class Apds9960I2CTransport
{
  public:
    Apds9960I2CTransport() {}
    ~Apds9960I2CTransport() {}

    struct Config
    {
        I2CHandle::Config::Peripheral periph;
        I2CHandle::Config::Speed      speed;
        dsy_gpio_pin                  scl;
        dsy_gpio_pin                  sda;

        Config()
        {
            periph = I2CHandle::Config::Peripheral::I2C_1;
            speed  = I2CHandle::Config::Speed::I2C_100KHZ;

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
        i2c_.TransmitBlocking(APDS9960_ADDRESS, data, size, 10);
    }

    void Read(uint8_t *data, uint16_t size)
    {
        i2c_.ReceiveBlocking(APDS9960_ADDRESS, data, size, 10);
    }

  private:
    I2CHandle i2c_;
};

/** @brief Device support for APDS9960
    gesture / RGB / proximity sensor.
    @author beserge
    @date December 2021
*/
template <typename Transport>
class Apds9960
{
  public:
    Apds9960() {}
    ~Apds9960() {}

    /** I2C Registers */
    enum
    {
        APDS9960_RAM        = 0x00,
        APDS9960_ENABLE     = 0x80,
        APDS9960_ATIME      = 0x81,
        APDS9960_WTIME      = 0x83,
        APDS9960_AILTIL     = 0x84,
        APDS9960_AILTH      = 0x85,
        APDS9960_AIHTL      = 0x86,
        APDS9960_AIHTH      = 0x87,
        APDS9960_PILT       = 0x89,
        APDS9960_PIHT       = 0x8B,
        APDS9960_PERS       = 0x8C,
        APDS9960_CONFIG1    = 0x8D,
        APDS9960_PPULSE     = 0x8E,
        APDS9960_CONTROL    = 0x8F,
        APDS9960_CONFIG2    = 0x90,
        APDS9960_ID         = 0x92,
        APDS9960_STATUS     = 0x93,
        APDS9960_CDATAL     = 0x94,
        APDS9960_CDATAH     = 0x95,
        APDS9960_RDATAL     = 0x96,
        APDS9960_RDATAH     = 0x97,
        APDS9960_GDATAL     = 0x98,
        APDS9960_GDATAH     = 0x99,
        APDS9960_BDATAL     = 0x9A,
        APDS9960_BDATAH     = 0x9B,
        APDS9960_PDATA      = 0x9C,
        APDS9960_POFFSET_UR = 0x9D,
        APDS9960_POFFSET_DL = 0x9E,
        APDS9960_CONFIG3    = 0x9F,
        APDS9960_GPENTH     = 0xA0,
        APDS9960_GEXTH      = 0xA1,
        APDS9960_GCONF1     = 0xA2,
        APDS9960_GCONF2     = 0xA3,
        APDS9960_GOFFSET_U  = 0xA4,
        APDS9960_GOFFSET_D  = 0xA5,
        APDS9960_GOFFSET_L  = 0xA7,
        APDS9960_GOFFSET_R  = 0xA9,
        APDS9960_GPULSE     = 0xA6,
        APDS9960_GCONF3     = 0xAA,
        APDS9960_GCONF4     = 0xAB,
        APDS9960_GFLVL      = 0xAE,
        APDS9960_GSTATUS    = 0xAF,
        APDS9960_IFORCE     = 0xE4,
        APDS9960_PICLEAR    = 0xE5,
        APDS9960_CICLEAR    = 0xE6,
        APDS9960_AICLEAR    = 0xE7,
        APDS9960_GFIFO_U    = 0xFC,
        APDS9960_GFIFO_D    = 0xFD,
        APDS9960_GFIFO_L    = 0xFE,
        APDS9960_GFIFO_R    = 0xFF,
    };

    struct Config
    {
        uint16_t integrationTimeMs;
        uint8_t  adcGain; // (0,3) -> {1x, 4x, 16x, 64x}

        uint8_t gestureDimensions; // (0,2) -> {all, up/down, left/right}
        uint8_t
                 gestureFifoThresh; // (0,3) -> interrupt after 1 dataset in fifo, 2, 3, 4
        uint8_t  gestureGain; // (0,3) -> {1x, 2x, 4x, 8x}
        uint16_t gestureProximityThresh;

        typename Transport::Config transport_config;

        Config()
        {
            integrationTimeMs = 10;
            adcGain           = 1; // 4x

            gestureDimensions      = 0; // gesture all
            gestureFifoThresh      = 1; // interrupt w/ 2 datasets in fifo
            gestureGain            = 2; // 4x gesture gain
            gestureProximityThresh = 50;
        }
    };

    /** Initialize the APDS9960 device
        \param config Configuration settings
    */
    void Init(Config config)
    {
        config_ = config;

        transport_.Init(config_.transport_config);

        /* Set default integration time and gain */
        SetADCIntegrationTime(config_.integrationTimeMs);
        SetADCGain(config.adcGain);

        // disable everything to start
        EnableGesture(false);
        EnableProximity(false);
        EnableColor(false);

        EnableColorInterrupt(false);
        EnableProximityInterrupt(false);
        ClearInterrupt();

        /* Note: by default, the device is in power down mode on bootup */
        Enable(false);
        System::Delay(10);
        Enable(true);
        System::Delay(10);

        // default to all gesture dimensions
        SetGestureDimensions(config_.gestureDimensions);
        SetGestureFIFOThreshold(config_.gestureFifoThresh);
        SetGestureGain(config_.gestureGain);
        SetGestureProximityThreshold(config_.gestureProximityThresh);
        ResetCounts();

        gpulse_.GPLEN  = 0x03; // 32 us
        gpulse_.GPULSE = 9;    // 10 pulses
        Write8(APDS9960_GPULSE, gpulse_.get());
    }

    /** Sets the integration time for the ADC of the APDS9960, in millis
        \param  iTimeMS Integration time
    */
    void SetADCIntegrationTime(uint16_t iTimeMS)
    {
        float temp;

        // convert ms into 2.78ms increments
        temp = iTimeMS;
        temp /= 2.78f;
        temp = 256.f - temp;
        if(temp > 255.f)
            temp = 255.f;
        if(temp < 0.f)
            temp = 0.f;

        /* Update the timing register */
        write8(APDS9960_ATIME, (uint8_t)temp);
    }

    /** Adjusts the color/ALS gain on the APDS9960 (adjusts the sensitivity to light)
        \param  aGain Gain
    */
    void SetADCGain(uint8_t aGain)
    {
        control_.AGAIN = aGain;
        write8(APDS9960_CONTROL, control_.get());
    }

    /** Sets gesture dimensions
        \param  dims Dimensions
    */
    void SetGestureDimensions(uint8_t dims)
    {
        Write8(APDS9960_GCONF3, dims & 0x03);
    }

    /** Sets gesture FIFO Threshold
        \param  thresh Threshold
    */
    void SetGestureFIFOThreshold(uint8_t thresh)
    {
        gconf1_.GFIFOTH = thresh;
        Write8(APDS9960_GCONF1, gconf1_.get());
    }

    /** Sets gesture sensor gain
        \param  gain Gain
    */
    void SetGestureGain(uint8_t gain)
    {
        gconf2_.GGAIN = gain;
        Write8(APDS9960_GCONF2, gconf2_.get());
    }

    /** Sets gesture sensor threshold
        \param  thresh Threshold
    */
    void SetGestureProximityThreshold(uint8_t thresh)
    {
        Write8(APDS9960_GPENTH, thresh);
    }

    /** Enables the device / Disables the device 
        (putting it in lower power sleep mode)
        \param  en Enable
    */
    void Enable(bool en)
    {
        enable_.PON = en;
        Write8(APDS9960_ENABLE, enable_.get());
    }

    /** Enable gesture readings
        \param  en Enable    
    */
    void EnableGesture(bool en)
    {
        if(!en)
        {
            gconf4_.GMODE = 0;
            write8(APDS9960_GCONF4, gconf4_.get());
        }
        enable_.GEN = en;
        write8(APDS9960_ENABLE, enable_.get());
        ResetCounts();
    }

    /** Enable proximity readings
        \param  en Enable
    */
    void EnableProximity(bool en)
    {
        enable_.PEN = en;

        write8(APDS9960_ENABLE, enable_.get());
    }

    /** Enable color readings
        \param  en Enable
    */
    void EnableColor(bool en)
    {
        enable_.AEN = en;
        write8(APDS9960_ENABLE, enable_.get());
    }

    /** Enables/disables color interrupt
        \param en Enable / disable
    */
    void EnableColorInterrupt(bool en)
    {
        enable_.AIEN = en;
        write8(APDS9960_ENABLE, enable_.get());
    }

    /** Enables / Disables color interrupt
        \param en Enable / disable
    */
    void EnableProximityInterrupt(bool en)
    {
        enable_.PIEN = en;
        write8(APDS9960_ENABLE, enable_.get());
    }

    /** Clears interrupt */
    void ClearInterrupt() { transport_.Write(APDS9960_AICLEAR, 1); }

    /** Resets gesture counts */
    void ResetCounts()
    {
        gestCnt_ = 0;
        UCount_  = 0;
        DCount_  = 0;
        LCount_  = 0;
        RCount_  = 0;
    }

    void Write8(uint8_t reg, uint8_t data)
    {
        uint8_t buff[2] = {reg, data};
        transport_.Write(buff, 2);
    }

    uint8_t Read8(uint8_t reg)
    {
        uint8_t buff[1] = {reg};
        transport_.Write(buff, 1);
        transport_.Read(buff, 1);
        return buff[0];
    }

    uint16_t Read16R(uint8_t reg)
    {
        uint8_t ret[2];
        transport_.Write(&reg, 1);
        transport_.Read(ret, 2);

        return (ret[1] << 8) | ret[0];
    }

    /** Read proximity data
        \return Proximity
    */
    uint8_t ReadProximity() { return read8(APDS9960_PDATA); }

    /** Returns validity status of a gesture
        \return Status (True/False)
    */
    bool GestureValid()
    {
        gstatus_.set(Read8(APDS9960_GSTATUS));
        return gstatus_.GVALID;
    }

    /** Reads gesture
        \return Received gesture (1,4) -> {UP, DOWN, LEFT, RIGHT}
    */
    uint8_t ReadGesture()
    {
        uint8_t       toRead;
        uint8_t       buf[256];
        unsigned long t = 0;
        uint8_t       gestureReceived;
        while(true)
        {
            int up_down_diff    = 0;
            int left_right_diff = 0;
            gestureReceived     = 0;
            if(!GestureValid())
                return 0;

            System::Delay(30);
            toRead = Read8(APDS9960_GFLVL);

            // produces sideffects needed for readGesture to work
            uint8_t reg = APDS9960_GFIFO_U;
            transport_.Write(&reg, 1);
            transport_.Read(buf, toRead);

            if(abs((int)buf[0] - (int)buf[1]) > 13)
                up_down_diff += (int)buf[0] - (int)buf[1];

            if(abs((int)buf[2] - (int)buf[3]) > 13)
                left_right_diff += (int)buf[2] - (int)buf[3];

            if(up_down_diff != 0)
            {
                if(up_down_diff < 0)
                {
                    if(DCount_ > 0)
                    {
                        gestureReceived = APDS9960_UP;
                    }
                    else
                        UCount_++;
                }
                else if(up_down_diff > 0)
                {
                    if(UCount_ > 0)
                    {
                        gestureReceived = APDS9960_DOWN;
                    }
                    else
                        DCount_++;
                }
            }

            if(left_right_diff != 0)
            {
                if(left_right_diff < 0)
                {
                    if(RCount_ > 0)
                    {
                        gestureReceived_ = APDS9960_LEFT;
                    }
                    else
                        LCount_++;
                }
                else if(left_right_diff > 0)
                {
                    if(LCount_ > 0)
                    {
                        gestureReceived_ = APDS9960_RIGHT;
                    }
                    else
                        RCount_++;
                }
            }

            if(up_down_diff != 0 || left_right_diff != 0)
                t = System::GetNow();

            if(gestureReceived || System::GetNow() - t > 300)
            {
                ResetCounts();
                return gestureReceived;
            }
        }
    }

    /** Set LED brightness for proximity/gesture
        \param  drive LED Drive (0,3) -> {100mA, 50mA, 25mA, 12.5mA}
        \param  boost LED Boost (0,3) -> {100%, 150%, 200%, 300%}
    */
    void SetLED(uint8_t drive, uint8_t boost)
    {
        config2_.LED_BOOST = boost;
        Write8(APDS9960_CONFIG2, config2_.get());

        control_.LDRIVE = drive;
        rite8(APDS9960_CONTROL, control_.get());
    }

    /** Returns status of color data
        \return True if color data ready, False otherwise
    */
    bool ColorDataReady()
    {
        status_.set(Read8(APDS9960_STATUS));
        return status_.AVALID;
    }

    /** Reads the raw red, green, blue and clear channel values
        \param  *r Red value
        \param  *g Green value
        \param  *b Blue value
        \param  *c Clear channel value
    */
    void GetColorData(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c)
    {
        *c = Read16R(APDS9960_CDATAL);
        *r = Read16R(APDS9960_RDATAL);
        *g = Read16R(APDS9960_GDATAL);
        *b = Read16R(APDS9960_BDATAL);
    }

  private:
    uint8_t gestCnt_, UCount_, DCount_, LCount_, RCount_; // counters
    uint8_t gestureReceived_;

    struct gconf1
    {
        uint8_t GEXPERS : 2; // Gesture Exit Persistence
        uint8_t GEXMSK : 4;  // Gesture Exit Mask
        uint8_t GFIFOTH : 2; // Gesture FIFO Threshold

        uint8_t get() { return (GFIFOTH << 6) | (GEXMSK << 2) | GEXPERS; }
    };

    gconf1 gconf1_;

    struct gconf2
    {
        uint8_t GWTIME : 3;  // Gesture Wait Time
        uint8_t GLDRIVE : 2; // Gesture LED Drive Strength
        uint8_t GGAIN : 2;   // Gesture Gain Control

        uint8_t get() { return (GGAIN << 5) | (GLDRIVE << 3) | GWTIME; }
    };
    gconf2 gconf2_;

    struct gconf4
    {
        uint8_t GMODE : 1; // Gesture mode
        uint8_t GIEN : 2;  // Gesture Interrupt Enable
        uint8_t get() { return (GIEN << 1) | GMODE; }
        void    set(uint8_t data)
        {
            GIEN  = (data >> 1) & 0x01;
            GMODE = data & 0x01;
        }
    };
    gconf4 gconf4_;

    struct control
    {
        uint8_t AGAIN : 2;  // ALS and Color gain control
        uint8_t PGAIN : 2;  // proximity gain control
        uint8_t LDRIVE : 2; // led drive strength

        uint8_t get() { return (LDRIVE << 6) | (PGAIN << 2) | AGAIN; }
    };
    control control_;

    struct enable
    {
        uint8_t PON : 1;  // power on
        uint8_t AEN : 1;  // ALS enable
        uint8_t PEN : 1;  // Proximity detect enable
        uint8_t WEN : 1;  // wait timer enable
        uint8_t AIEN : 1; // ALS interrupt enable
        uint8_t PIEN : 1; // proximity interrupt enable
        uint8_t GEN : 1;  // gesture enable

        uint8_t get()
        {
            return (GEN << 6) | (PIEN << 5) | (AIEN << 4) | (WEN << 3)
                   | (PEN << 2) | (AEN << 1) | PON;
        };
    };
    struct enable enable_;

    struct gpulse
    {
        uint8_t GPULSE : 6; // Number of gesture pulses = GPULSE + 1
        uint8_t GPLEN : 2;  // Gesture Pulse Length

        uint8_t get() { return (GPLEN << 6) | GPULSE; }
    };
    gpulse gpulse_;

    struct gstatus
    {
        uint8_t GVALID : 1; // Gesture FIFO Data Are we OK to read FIFO?
        uint8_t GFOV : 1;   // Gesture FIFO Overflow Flag

        void set(uint8_t data)
        {
            GFOV   = (data >> 1) & 0x01;
            GVALID = data & 0x01;
        }
    };
    gstatus gstatus_;

    struct config2
    {
        /* Additional LDR current during proximity and gesture LED pulses. Current
        value, set by LDRIVE, is increased by the percentage of LED_BOOST.
        */
        uint8_t LED_BOOST : 2;
        uint8_t CPSIEN : 1; // clear photodiode saturation int enable
        uint8_t PSIEN : 1;  // proximity saturation interrupt enable

        uint8_t get()
        {
            return (PSIEN << 7) | (CPSIEN << 6) | (LED_BOOST << 4) | 1;
        }
    };
    config2 config2_;

    struct status
    {
        uint8_t AVALID : 1; // ALS Valid
        uint8_t PVALID : 1; // Proximity Valid
        uint8_t GINT : 1;   // Gesture Interrupt
        uint8_t AINT : 1;   // ALS Interrupt
        uint8_t PINT : 1;   // Proximity Interrupt

        /* Indicates that an analog saturation event occurred during a previous
        proximity or gesture cycle. Once set, this bit remains set until cleared by
        clear proximity interrupt special function command (0xE5 PICLEAR) or by
        disabling Prox (PEN=0). This bit triggers an interrupt if PSIEN is set.
        */
        uint8_t PGSAT : 1;

        /* Clear Photodiode Saturation. When asserted, the analog sensor was at the
        upper end of its dynamic range. The bit can be de-asserted by sending a
        Clear channel interrupt command (0xE6 CICLEAR) or by disabling the ADC
        (AEN=0). This bit triggers an interrupt if CPSIEN is set.
        */
        uint8_t CPSAT : 1;

        void set(uint8_t data)
        {
            AVALID = data & 0x01;
            PVALID = (data >> 1) & 0x01;
            GINT   = (data >> 2) & 0x01;
            AINT   = (data >> 4) & 0x01;
            PINT   = (data >> 5) & 0x01;
            PGSAT  = (data >> 6) & 0x01;
            CPSAT  = (data >> 7) & 0x01;
        }
    };
    status status_;

    Config    config_;
    Transport transport_;
};

/** @} */

using Apds9960I2C = Apds9960<Apds9960I2CTransport>;
} // namespace daisy
#endif
