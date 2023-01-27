#pragma once
#ifndef DSY_APDS9960_H
#define DSY_APDS9960_H

#define APDS9960_ADDRESS (0x39) /**< I2C Address */

#define APDS9960_UP 0x01    /**< Gesture Up */
#define APDS9960_DOWN 0x02  /**< Gesture Down */
#define APDS9960_LEFT 0x03  /**< Gesture Left */
#define APDS9960_RIGHT 0x04 /**< Gesture Right */

/** I2C Registers */
#define APDS9960_RAM 0x00
#define APDS9960_ENABLE 0x80
#define APDS9960_ATIME 0x81
#define APDS9960_WTIME 0x83
#define APDS9960_AILTIL 0x84
#define APDS9960_AILTH 0x85
#define APDS9960_AIHTL 0x86
#define APDS9960_AIHTH 0x87
#define APDS9960_PILT 0x89
#define APDS9960_PIHT 0x8B
#define APDS9960_PERS 0x8C
#define APDS9960_CONFIG1 0x8D
#define APDS9960_PPULSE 0x8E
#define APDS9960_CONTROL 0x8F
#define APDS9960_CONFIG2 0x90
#define APDS9960_ID 0x92
#define APDS9960_STATUS 0x93
#define APDS9960_CDATAL 0x94
#define APDS9960_CDATAH 0x95
#define APDS9960_RDATAL 0x96
#define APDS9960_RDATAH 0x97
#define APDS9960_GDATAL 0x98
#define APDS9960_GDATAH 0x99
#define APDS9960_BDATAL 0x9A
#define APDS9960_BDATAH 0x9B
#define APDS9960_PDATA 0x9C
#define APDS9960_POFFSET_UR 0x9D
#define APDS9960_POFFSET_DL 0x9E
#define APDS9960_CONFIG3 0x9F
#define APDS9960_GPENTH 0xA0
#define APDS9960_GEXTH 0xA1
#define APDS9960_GCONF1 0xA2
#define APDS9960_GCONF2 0xA3
#define APDS9960_GOFFSET_U 0xA4
#define APDS9960_GOFFSET_D 0xA5
#define APDS9960_GOFFSET_L 0xA7
#define APDS9960_GOFFSET_R 0xA9
#define APDS9960_GPULSE 0xA6
#define APDS9960_GCONF3 0xAA
#define APDS9960_GCONF4 0xAB
#define APDS9960_GFLVL 0xAE
#define APDS9960_GSTATUS 0xAF
#define APDS9960_IFORCE 0xE4
#define APDS9960_PICLEAR 0xE5
#define APDS9960_CICLEAR 0xE6
#define APDS9960_AICLEAR 0xE7
#define APDS9960_GFIFO_U 0xFC
#define APDS9960_GFIFO_D 0xFD
#define APDS9960_GFIFO_L 0xFE
#define APDS9960_GFIFO_R 0xFF

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
        Pin                           scl;
        Pin                           sda;

        Config()
        {
            periph = I2CHandle::Config::Peripheral::I2C_1;
            speed  = I2CHandle::Config::Speed::I2C_100KHZ;

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
    bool Write(uint8_t *data, uint16_t size)
    {
        return I2CHandle::Result::OK
               != i2c_.TransmitBlocking(APDS9960_ADDRESS, data, size, 10);
    }

    /** \return Did the transaction error? i.e. Return true if error, false if ok */
    bool Read(uint8_t *data, uint16_t size)
    {
        return I2CHandle::Result::OK
               != i2c_.ReceiveBlocking(APDS9960_ADDRESS, data, size, 10);
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

    struct Config
    {
        uint16_t integrationTimeMs;
        uint8_t  adcGain; // (0,3) -> {1x, 4x, 16x, 64x}

        uint8_t gestureDimensions; // (0,2) -> {all, up/down, left/right}
        uint8_t
                 gestureFifoThresh; // (0,3) -> interrupt after 1 dataset in fifo, 2, 3, 4
        uint8_t  gestureGain; // (0,3) -> {1x, 2x, 4x, 8x}
        uint16_t gestureProximityThresh;

        bool color_mode;
        bool prox_mode;
        bool gesture_mode;

        typename Transport::Config transport_config;

        Config()
        {
            integrationTimeMs = 10;
            adcGain           = 1; // 4x

            gestureDimensions      = 0; // gesture all
            gestureFifoThresh      = 1; // interrupt w/ 2 datasets in fifo
            gestureGain            = 2; // 4x gesture gain
            gestureProximityThresh = 40;

            color_mode   = true;
            prox_mode    = true;
            gesture_mode = false;
        }
    };

    enum Result
    {
        OK = 0,
        ERR
    };

    // turn on/off elements
    void enable(bool en = true);
    /** Initialize the APDS9960 device
        \param config Configuration settings
    */
    Result Init(Config config)
    {
        config_          = config;
        transport_error_ = false;

        SetTransportErr(transport_.Init(config_.transport_config));

        /* Set default integration time and gain */
        SetADCIntegrationTime(config_.integrationTimeMs);
        SetADCGain(config.adcGain);

        // disable everything to start
        EnableGesture(false);
        EnableProximity(false);
        EnableColor(false);

        SetColorInterrupt(false);
        SetProximityInterrupt(false);
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

        // prox / color mode by default
        // only one gesture or prox can be used at a time
        // in gesture mode you should have prox mode on also, the data will just not be useful
        EnableGesture(config_.gesture_mode);
        EnableProximity(config_.prox_mode);
        EnableColor(config_.color_mode);

        return GetTransportErr();
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
        Write8(APDS9960_ATIME, (uint8_t)temp);
    }

    /**
    *    Returns the integration time for the ADC of the APDS9960, in millis
    *  \return Integration time
    */
    float GetADCIntegrationTime()
    {
        float temp;

        temp = Read8(APDS9960_ATIME);

        // convert to units of 2.78 ms
        temp = 256 - temp;
        temp *= 2.78;
        return temp;
    }

    /** Adjusts the color/ALS gain on the APDS9960 (adjusts the sensitivity to light)
        \param  aGain Gain
    */
    void SetADCGain(uint8_t aGain)
    {
        control_.AGAIN = aGain;
        Write8(APDS9960_CONTROL, control_.get());
    }

    /** Sets gesture sensor offset
        \param  offset_up Up offset  
        \param  offset_down Down offset
        \param  offset_left Left offset
        \param  offset_right Right offset
    */
    void SetGestureOffset(uint8_t offset_up,
                          uint8_t offset_down,
                          uint8_t offset_left,
                          uint8_t offset_right)
    {
        Write8(APDS9960_GOFFSET_U, offset_up);
        Write8(APDS9960_GOFFSET_D, offset_down);
        Write8(APDS9960_GOFFSET_L, offset_left);
        Write8(APDS9960_GOFFSET_R, offset_right);
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
            Write8(APDS9960_GCONF4, gconf4_.get());
        }
        enable_.GEN = en;
        Write8(APDS9960_ENABLE, enable_.get());
        ResetCounts();
    }

    /** Enable proximity readings
        \param  en Enable
    */
    void EnableProximity(bool en)
    {
        enable_.PEN = en;

        Write8(APDS9960_ENABLE, enable_.get());
    }

    /** Enable color readings
        \param  en Enable
    */
    void EnableColor(bool en)
    {
        enable_.AEN = en;
        Write8(APDS9960_ENABLE, enable_.get());
    }

    /** Enables/disables color interrupt
        \param en Enable / disable
    */
    void SetColorInterrupt(bool en)
    {
        enable_.AIEN = en;
        Write8(APDS9960_ENABLE, enable_.get());
    }

    /** Enables / Disables color interrupt
        \param en Enable / disable
    */
    void SetProximityInterrupt(bool en)
    {
        enable_.PIEN = en;
        Write8(APDS9960_ENABLE, enable_.get());
    }

    /** Clears interrupt */
    void ClearInterrupt()
    {
        uint8_t val = APDS9960_AICLEAR;
        SetTransportErr(transport_.Write(&val, 1));
    }

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
        SetTransportErr(transport_.Write(buff, 2));
    }

    uint8_t Read8(uint8_t reg)
    {
        uint8_t buff[1] = {reg};
        SetTransportErr(transport_.Write(buff, 1));
        SetTransportErr(transport_.Read(buff, 1));
        return buff[0];
    }

    uint16_t Read16R(uint8_t reg)
    {
        uint8_t ret[2];
        SetTransportErr(transport_.Write(&reg, 1));
        SetTransportErr(transport_.Read(ret, 2));

        return (ret[1] << 8) | ret[0];
    }

    /** Read proximity data
        \return Proximity
    */
    uint8_t ReadProximity() { return Read8(APDS9960_PDATA); }

    /** Adjusts the Proximity gain on the APDS9960
        \param  pGain Gain
    */
    void SetProxGain(uint8_t pGain)
    {
        control_.PGAIN = pGain;

        /* Update the timing register */
        Write8(APDS9960_CONTROL, control_.get());
    }

    /** Returns the Proximity gain on the APDS9960
        \return Proxmity gain
    */
    uint8_t GetProxGain() { return ((Read8(APDS9960_CONTROL) & 0x0C) >> 2); }

    /** Sets number of proxmity pulses
        \param  pLen Pulse Length
        \param  pulses Number of pulses
    */
    void SetProxPulse(uint8_t pLen, uint8_t pulses)
    {
        if(pulses < 1)
            pulses = 1;
        if(pulses > 64)
            pulses = 64;
        pulses--;

        ppulse_.PPLEN  = pLen;
        ppulse_.PPULSE = pulses;

        Write8(APDS9960_PPULSE, ppulse_.get());
    }

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
            SetTransportErr(transport_.Write(&reg, 1));
            SetTransportErr(transport_.Read(buf, toRead));

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

    /** Converts the raw R/G/B values to color temperature in degrees Kelvin
        \param  r Red value
        \param  g Green value
        \param  b Blue value
        
        \return Color temperature
    */
    uint16_t CalculateColorTemperature(uint16_t r, uint16_t g, uint16_t b)
    {
        float X, Y, Z; /* RGB to XYZ correlation      */
        float xc, yc;  /* Chromaticity co-ordinates   */
        float n;       /* McCamy's formula            */
        float cct;

        /* 1. Map RGB values to their XYZ counterparts.    */
        /* Based on 6500K fluorescent, 3000K fluorescent   */
        /* and 60W incandescent values for a wide range.   */
        /* Note: Y = Illuminance or lux                    */
        X = (-0.14282F * r) + (1.54924F * g) + (-0.95641F * b);
        Y = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);
        Z = (-0.68202F * r) + (0.77073F * g) + (0.56332F * b);

        /* 2. Calculate the chromaticity co-ordinates      */
        xc = (X) / (X + Y + Z);
        yc = (Y) / (X + Y + Z);

        /* 3. Use McCamy's formula to determine the CCT    */
        n = (xc - 0.3320F) / (0.1858F - yc);

        /* Calculate the final CCT */
        cct = (449.0F * powf(n, 3)) + (3525.0F * powf(n, 2)) + (6823.3F * n)
              + 5520.33F;

        /* Return the results in degrees Kelvin */
        return (uint16_t)cct;
    }

    /** Calculate ambient light values
        \param  r Red value
        \param  g Green value
        \param  b Blue value
        \return LUX value
    */
    uint16_t CalculateLux(uint16_t r, uint16_t g, uint16_t b)
    {
        float illuminance;

        /* This only uses RGB ... how can we integrate clear or calculate lux */
        /* based exclusively on clear since this might be more reliable?      */
        illuminance = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);

        return (uint16_t)illuminance;
    }

    /** Sets interrupt limits
        \param  low Low limit
        \param  high High limit
    */
    void SetIntLimits(uint16_t low, uint16_t high)
    {
        Write8(APDS9960_AILTIL, low & 0xFF);
        Write8(APDS9960_AILTH, low >> 8);
        Write8(APDS9960_AIHTL, high & 0xFF);
        Write8(APDS9960_AIHTH, high >> 8);
    }


    /** Returns status of color data
        \return True if color data ready, False otherwise
    */
    bool ColorDataReady()
    {
        status_.set(Read8(APDS9960_STATUS));
        return status_.AVALID;
    }


    /** Reads the raw red channel value
        \return Red channel value
    */
    uint16_t GetColorDataRed() { return Read16R(APDS9960_RDATAL); }


    /** Reads the raw green channel value
        \return Green channel value
    */
    uint16_t GetColorDataGreen() { return Read16R(APDS9960_GDATAL); }


    /** Reads the raw blue channel value
        \return Blue channel value
    */
    uint16_t GetColorDataBlue() { return Read16R(APDS9960_BDATAL); }


    /** Reads the raw clear channel value
        \return Clear channel value
    */
    uint16_t GetColorDataClear() { return Read16R(APDS9960_CDATAL); }


    /** Reads the raw red, green, blue and clear channel values
        \param  *r Red value
        \param  *g Green value
        \param  *b Blue value
        \param  *c Clear channel value
    */
    void GetColorData(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c)
    {
        *c = GetColorDataClear();
        *r = GetColorDataRed();
        *g = GetColorDataGreen();
        *b = GetColorDataBlue();
    }

  private:
    uint8_t gestCnt_, UCount_, DCount_, LCount_, RCount_; // counters
    uint8_t gestureReceived_;
    bool    transport_error_;

    Config    config_;
    Transport transport_;

    /** Set the global transport_error_ bool */
    void SetTransportErr(bool err) { transport_error_ |= err; }

    /** Get the global transport_error_ bool (as a Result), then reset it */
    Result GetTransportErr()
    {
        Result ret       = transport_error_ ? ERR : OK;
        transport_error_ = false;
        return ret;
    }

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

    struct ppulse
    {
        uint8_t PPULSE : 6; //Proximity Pulse Count
        uint8_t PPLEN : 2;  // Proximity Pulse Length

        uint8_t get() { return (PPLEN << 6) | PPULSE; }
    };
    ppulse ppulse_;

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
};

/** @} */

using Apds9960I2C = Apds9960<Apds9960I2CTransport>;
} // namespace daisy
#endif
