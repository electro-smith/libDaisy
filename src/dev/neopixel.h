#pragma once
#ifndef DSY_NEO_PIXEL_H
#define DSY_NEO_PIXEL_H

#define SEESAW_ADDRESS (0x49) ///< Default Seesaw I2C address

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** I2C Transport for NeoPixel */
class NeoPixelI2CTransport
{
  public:
    NeoPixelI2CTransport() {}
    ~NeoPixelI2CTransport() {}

    struct Config
    {
        I2CHandle::Config::Peripheral periph;
        I2CHandle::Config::Speed      speed;
        dsy_gpio_pin                  scl;
        dsy_gpio_pin                  sda;

        uint8_t address;

        Config()
        {
            address = SEESAW_ADDRESS;

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

        error_ |= I2CHandle::Result::OK != i2c_.Init(i2c_config);
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

    void
    ReadLen(uint8_t reg_high, uint8_t reg_low, uint8_t *buff, uint16_t size)
    {
        uint8_t reg[2] = {reg_high, reg_low};
        Write(reg, 2);
        Read(buff, size);
    }

    void
    WriteLen(uint8_t reg_high, uint8_t reg_low, uint8_t *buff, uint16_t size)
    {
        // max write size of 126...
        if(size >= 126)
        {
            return;
        }

        uint8_t reg[128];

        reg[0] = reg_high;
        reg[1] = reg_low;

        for(int i = 0; i < size; i++)
        {
            reg[i] = buff[i + 2];
        }

        Write(reg, size + 2);
    }

    /**  Writes an 8 bit value
        \param reg the register address to write to
        \param value the value to write to the register
    */
    void Write8(uint8_t reg_high, uint8_t reg_low, uint8_t value)
    {
        uint8_t buffer[3];

        buffer[0] = reg_high;
        buffer[1] = reg_low;
        buffer[2] = value;

        Write(buffer, 3);
    }

    /**  Reads an 8 bit value
        \param reg the register address to read from
        \returns the 16 bit data value read from the device
    */
    uint8_t Read8(uint8_t reg_high, uint8_t reg_low)
    {
        uint8_t buffer;
        ReadLen(reg_high, reg_low, &buffer, 1);
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
class NeoPixel
{
  public:
    NeoPixel() {}
    ~NeoPixel() {}

    struct Config
    {
        typename Transport::Config transport_config;
        bool                       init_reset;

        Config() { init_reset = true; }
    };

    enum Result
    {
        OK = 0,
        ERR
    };

    typedef uint16_t neoPixelType;

    /** Module Base Addreses
        The module base addresses for different seesaw modules.
    */
    enum ModBaseAdd
    {
        SEESAW_STATUS_BASE  = 0x00,
        SEESAW_GPIO_BASE    = 0x01,
        SEESAW_SERCOM0_BASE = 0x02,

        SEESAW_TIMER_BASE     = 0x08,
        SEESAW_ADC_BASE       = 0x09,
        SEESAW_DAC_BASE       = 0x0A,
        SEESAW_INTERRUPT_BASE = 0x0B,
        SEESAW_DAP_BASE       = 0x0C,
        SEESAW_EEPROM_BASE    = 0x0D,
        SEESAW_NEOPIXEL_BASE  = 0x0E,
        SEESAW_TOUCH_BASE     = 0x0F,
        SEESAW_KEYPAD_BASE    = 0x10,
        SEESAW_ENCODER_BASE   = 0x11,
        SEESAW_SPECTRUM_BASE  = 0x12,
    };

    /** neopixel module function address registers */
    enum ModAddReg
    {
        SEESAW_NEOPIXEL_STATUS     = 0x00,
        SEESAW_NEOPIXEL_PIN        = 0x01,
        SEESAW_NEOPIXEL_SPEED      = 0x02,
        SEESAW_NEOPIXEL_BUF_LENGTH = 0x03,
        SEESAW_NEOPIXEL_BUF        = 0x04,
        SEESAW_NEOPIXEL_SHOW       = 0x05,
    };

    /** status module function address registers */
    enum StatAddReg
    {
        SEESAW_STATUS_HW_ID   = 0x01,
        SEESAW_STATUS_VERSION = 0x02,
        SEESAW_STATUS_OPTIONS = 0x03,
        SEESAW_STATUS_TEMP    = 0x04,
        SEESAW_STATUS_SWRST   = 0x7F,
    };

    /** Initialize the NeoPixel device
        \param config Configuration settings
    */
    Result Init(Config config)
    {
        config_ = config;

        transport_.Init(config_.transport_config);

        if(config_.init_reset)
            SWReset();

        UpdateType(type);
        UpdateLength(numLEDs);
        SetPin(pin);

        return GetTransportError();
    }

    void Write(uint8_t reg_high, uint8_t reg_low, uint8_t *buff, uint8_t size)
    {
        transport_.WriteLen(reg_high, reg_low, buff, size);
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
    uint8_t Read8(uint8_t reg_high, uint8_t reg_low)
    {
        return transport_.Read8(reg_high, reg_low);
    }

    void ReadLen(uint8_t reg_high, uint8_t reg_low, uint8_t *buff, uint8_t len)
    {
        transport_.ReadLen(reg_high, reg_low, buff, len);
    }

    /** Get and reset the transport error flag
        \return Whether the transport has errored since the last check
    */
    Result GetTransportError() { return transport_.GetError() ? ERR : OK; }

    /** Perform a software reset. 
        This resets all seesaw registers to their default values.
        This is called automatically from Init().
    */
    void SWReset()
    {
        return Write8(SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST);
        System::Delay(255);
    }

    void UpdateLength(uint16_t n)
    {
        if(pixels)
            free(pixels); // Free existing data (if any)

        // Allocate new data -- note: ALL PIXELS ARE CLEARED
        numBytes = n * ((wOffset == rOffset) ? 3 : 4);
        if((pixels = (uint8_t *)malloc(numBytes)))
        {
            memset(pixels, 0, numBytes);
            numLEDs = n;
        }
        else
        {
            numLEDs = numBytes = 0;
        }

        uint8_t buf[] = {(uint8_t)(numBytes >> 8), (uint8_t)(numBytes & 0xFF)};
        Write(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF_LENGTH, buf, 2);
    }

    void UpdateType(neoPixelType t)
    {
        bool oldThreeBytesPerPixel = (wOffset == rOffset); // false if RGBW

        wOffset  = (t >> 6) & 0b11; // See notes in header file
        rOffset  = (t >> 4) & 0b11; // regarding R/G/B/W offsets
        gOffset  = (t >> 2) & 0b11;
        bOffset  = t & 0b11;
        is800KHz = (t < 256); // 400 KHz flag is 1<<8

        Write8(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_SPEED, is800KHz);

        // If bytes-per-pixel has changed (and pixel data was previously
        // allocated), re-allocate to new size.  Will clear any data.
        if(pixels)
        {
            bool newThreeBytesPerPixel = (wOffset == rOffset);
            if(newThreeBytesPerPixel != oldThreeBytesPerPixel)
                UpdateLength(numLEDs);
        }
    }

    inline bool CanShow(void) { return (System::GetUs() - endTime) >= 300L; }

    void Show(void)
    {
        if(!pixels)
            return;

        // Data latch = 300+ microsecond pause in the output stream.  Rather than
        // put a delay at the end of the function, the ending time is noted and
        // the function will simply hold off (if needed) on issuing the
        // subsequent round of data until the latch time has elapsed.  This
        // allows the mainline code to start generating the next frame of data
        // rather than stalling for the latch.
        while(!CanShow())
            ;

        Write(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_SHOW, NULL, 0);

        endTime = System::GetUs(); // Save EOD time for latch on next call
    }

    // Set the output pin number
    void SetPin(uint8_t p)
    {
        Write8(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_PIN, p);
        pin = p;
    }

    // Set pixel color from separate R,G,B components:
    void SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
    {
        if(n < numLEDs)
        {
            if(brightness)
            { // See notes in setBrightness()
                r = (r * brightness) >> 8;
                g = (g * brightness) >> 8;
                b = (b * brightness) >> 8;
            }
            uint8_t *p;
            if(wOffset == rOffset)
            {                       // Is an RGB-type strip
                p = &pixels[n * 3]; // 3 bytes per pixel
            }
            else
            {                                // Is a WRGB-type strip
                p          = &pixels[n * 4]; // 4 bytes per pixel
                p[wOffset] = 0; // But only R,G,B passed -- set W to 0
            }
            p[rOffset] = r; // R,G,B always stored
            p[gOffset] = g;
            p[bOffset] = b;

            uint8_t  len    = (wOffset == rOffset ? 3 : 4);
            uint16_t offset = n * len;

            uint8_t writeBuf[6];
            writeBuf[0] = (offset >> 8);
            writeBuf[1] = offset;
            mymemcpy(&writeBuf[2], p, len);

            Write(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF, writeBuf, len + 2);
        }
    }

    void SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
    {
        if(n < numLEDs)
        {
            if(brightness)
            { // See notes in setBrightness()
                r = (r * brightness) >> 8;
                g = (g * brightness) >> 8;
                b = (b * brightness) >> 8;
                w = (w * brightness) >> 8;
            }
            uint8_t *p;
            if(wOffset == rOffset)
            {                       // Is an RGB-type strip
                p = &pixels[n * 3]; // 3 bytes per pixel (ignore W)
            }
            else
            {                                // Is a WRGB-type strip
                p          = &pixels[n * 4]; // 4 bytes per pixel
                p[wOffset] = w;              // Store W
            }
            p[rOffset] = r; // Store R,G,B
            p[gOffset] = g;
            p[bOffset] = b;

            uint8_t  len    = (wOffset == rOffset ? 3 : 4);
            uint16_t offset = n * len;

            uint8_t writeBuf[6];
            writeBuf[0] = (offset >> 8);
            writeBuf[1] = offset;
            mymemcpy(&writeBuf[2], p, len);

            Write(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF, writeBuf, len + 2);
        }
    }

    // Set pixel color from 'packed' 32-bit RGB color:
    void SetPixelColor(uint16_t n, uint32_t c)
    {
        if(n < numLEDs)
        {
            uint8_t *p, r = (uint8_t)(c >> 16), g = (uint8_t)(c >> 8),
                        b = (uint8_t)c;
            if(brightness)
            { // See notes in setBrightness()
                r = (r * brightness) >> 8;
                g = (g * brightness) >> 8;
                b = (b * brightness) >> 8;
            }
            if(wOffset == rOffset)
            {
                p = &pixels[n * 3];
            }
            else
            {
                p          = &pixels[n * 4];
                uint8_t w  = (uint8_t)(c >> 24);
                p[wOffset] = brightness ? ((w * brightness) >> 8) : w;
            }
            p[rOffset] = r;
            p[gOffset] = g;
            p[bOffset] = b;

            uint8_t  len    = (wOffset == rOffset ? 3 : 4);
            uint16_t offset = n * len;

            uint8_t writeBuf[6];
            writeBuf[0] = (offset >> 8);
            writeBuf[1] = offset;
            mymemcpy(&writeBuf[2], p, len);

            Write(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF, writeBuf, len + 2);
        }
    }

    // Convert separate R,G,B into packed 32-bit RGB color.
    // Packed format is always RGB, regardless of LED strand color order.
    uint32_t Color(uint8_t r, uint8_t g, uint8_t b)
    {
        return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

    // Convert separate R,G,B,W into packed 32-bit WRGB color.
    // Packed format is always WRGB, regardless of LED strand color order.
    uint32_t Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
    {
        return ((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8)
               | b;
    }

    // Query color from previously-set pixel (returns packed 32-bit RGB value)
    uint32_t GetPixelColor(uint16_t n) const
    {
        if(n >= numLEDs)
            return 0; // Out of bounds, return no color.

        uint8_t *p;

        if(wOffset == rOffset)
        { // Is RGB-type device
            p = &pixels[n * 3];
            if(brightness)
            {
                // Stored color was decimated by setBrightness().  Returned value
                // attempts to scale back to an approximation of the original 24-bit
                // value used when setting the pixel color, but there will always be
                // some error -- those bits are simply gone.  Issue is most
                // pronounced at low brightness levels.
                return (((uint32_t)(p[rOffset] << 8) / brightness) << 16)
                       | (((uint32_t)(p[gOffset] << 8) / brightness) << 8)
                       | ((uint32_t)(p[bOffset] << 8) / brightness);
            }
            else
            {
                // No brightness adjustment has been made -- return 'raw' color
                return ((uint32_t)p[rOffset] << 16)
                       | ((uint32_t)p[gOffset] << 8) | (uint32_t)p[bOffset];
            }
        }
        else
        { // Is RGBW-type device
            p = &pixels[n * 4];
            if(brightness)
            { // Return scaled color
                return (((uint32_t)(p[wOffset] << 8) / brightness) << 24)
                       | (((uint32_t)(p[rOffset] << 8) / brightness) << 16)
                       | (((uint32_t)(p[gOffset] << 8) / brightness) << 8)
                       | ((uint32_t)(p[bOffset] << 8) / brightness);
            }
            else
            { // Return raw color
                return ((uint32_t)p[wOffset] << 24)
                       | ((uint32_t)p[rOffset] << 16)
                       | ((uint32_t)p[gOffset] << 8) | (uint32_t)p[bOffset];
            }
        }
    }

    // Returns pointer to pixels[] array.  Pixel data is stored in device-
    // native format and is not translated here.  Application will need to be
    // aware of specific pixel data format and handle colors appropriately.
    uint8_t *GetPixels(void) const { return pixels; }

    uint16_t NumPixels(void) const { return numLEDs; }

    void Clear()
    {
        // Clear local pixel buffer
        memset(pixels, 0, numBytes);

        // Now clear the pixels on the seesaw
        uint8_t writeBuf[32];
        memset(writeBuf, 0, 32);
        for(uint8_t offset = 0; offset < numBytes; offset += 32 - 4)
        {
            writeBuf[0] = (offset >> 8);
            writeBuf[1] = offset;
            Write(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF, writeBuf, 32);
        }
    }

    void SetBrightness(uint8_t b) { brightness = b; }

  private:
    void mymemcpy(uint8_t *dest, uint8_t *src, uint8_t len)
    {
        for(uint8_t i = 0; i < len; i++)
        {
            dest[i] = src[i];
        }
    }

    Config    config_;
    Transport transport_;

  protected:
    bool is800KHz,    // ...true if 800 KHz pixels
        begun;        // true if begin() previously called
    uint16_t numLEDs, // Number of RGB LEDs in strip
        numBytes;     // Size of 'pixels' buffer below (3 or 4 bytes/pixel)
    int8_t  pin;
    uint8_t brightness,
        *pixels,      // Holds LED color values (3 or 4 bytes each)
        rOffset,      // Index of red byte within each 3- or 4-byte pixel
        gOffset,      // Index of green byte
        bOffset,      // Index of blue byte
        wOffset;      // Index of white byte (same as rOffset if no white)
    uint32_t endTime; // Latch timing reference

    uint16_t type;

}; // namespace daisy

/** @} */

using NeoPixelI2C = NeoPixel<NeoPixelI2CTransport>;
} // namespace daisy
#endif