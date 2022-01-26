#pragma once
#ifndef DSY_NEO_TRELLIS_H
#define DSY_NEO_TRELLIS_H

#include "dev/neopixel.h"

#define NEO_TRELLIS_ADDR 0x2E

#define NEO_TRELLIS_NEOPIX_PIN 3

#define NEO_TRELLIS_NUM_ROWS 4
#define NEO_TRELLIS_NUM_COLS 4
#define NEO_TRELLIS_NUM_KEYS (NEO_TRELLIS_NUM_ROWS * NEO_TRELLIS_NUM_COLS)

#define NEO_TRELLIS_MAX_CALLBACKS 32

#define NEO_TRELLIS_KEY(x) (((x) / 4) * 8 + ((x) % 4))
#define NEO_TRELLIS_SEESAW_KEY(x) (((x) / 8) * 4 + ((x) % 8))

#define NEO_TRELLIS_X(k) ((k) % 4)
#define NEO_TRELLIS_Y(k) ((k) / 4)

#define NEO_TRELLIS_XY(x, y) ((y)*NEO_TRELLIS_NUM_ROWS + (x))

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** I2C Transport for NeoTrellis */
class NeoTrellisI2CTransport
{
  public:
    NeoTrellisI2CTransport() {}
    ~NeoTrellisI2CTransport() {}

    struct Config
    {
        I2CHandle::Config::Peripheral periph;
        I2CHandle::Config::Speed      speed;
        Pin                           scl;
        Pin                           sda;

        uint8_t address;

        Config()
        {
            address = NEO_TRELLIS_ADDR;

            periph = I2CHandle::Config::Peripheral::I2C_1;
            speed  = I2CHandle::Config::Speed::I2C_400KHZ;

            scl = Pin(PORTB, 8);
            sda = Pin(PORTB, 9);
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

    void ReadLen(uint8_t  reg_high,
                 uint8_t  reg_low,
                 uint8_t *buff,
                 uint16_t size,
                 int      delay)
    {
        uint8_t reg[2] = {reg_high, reg_low};
        Write(reg, 2);

        System::DelayUs(delay);

        Read(buff, size);
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
    uint8_t Read8(uint8_t reg_high, uint8_t reg_low, int delay)
    {
        uint8_t buffer;
        ReadLen(reg_high, reg_low, &buffer, 1, delay);
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

/** \brief Device support for the Adafruit Neotrellis device
    @author beserge
    @date December 2021
*/
template <typename Transport>
class NeoTrellis
{
  public:
    NeoTrellis() {}
    ~NeoTrellis() {}

    /** Module Base Addreses
        The module base addresses for different seesaw modules.
    */
    enum ModuleBaseAddress
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


    /** keypad module function address registers */
    enum KeypadFuncAddRegs
    {
        SEESAW_KEYPAD_STATUS   = 0x00,
        SEESAW_KEYPAD_EVENT    = 0x01,
        SEESAW_KEYPAD_INTENSET = 0x02,
        SEESAW_KEYPAD_INTENCLR = 0x03,
        SEESAW_KEYPAD_COUNT    = 0x04,
        SEESAW_KEYPAD_FIFO     = 0x10,
    };

    /** status module function address registers */
    enum StatusFuncAddRegs
    {
        SEESAW_STATUS_HW_ID   = 0x01,
        SEESAW_STATUS_VERSION = 0x02,
        SEESAW_STATUS_OPTIONS = 0x03,
        SEESAW_STATUS_TEMP    = 0x04,
        SEESAW_STATUS_SWRST   = 0x7F,
    };

    union keyEventRaw
    {
        struct
        {
            uint8_t EDGE : 2; ///< the edge that was triggered
            uint8_t NUM : 6;  ///< the event number
        } bit;                ///< bitfield format
        uint8_t reg;          ///< register format
    };

    /** extended key event stucture for keypad module */
    union keyEvent
    {
        struct Bit
        {
            uint8_t  EDGE : 2; ///< the edge that was triggered
            uint16_t NUM : 14; ///< the event number
        } bit;                 ///< bitfield format
        uint16_t reg;          ///< register format
    };

    /** key state struct that will be written to seesaw chip keypad module */
    union keyState
    {
        struct
        {
            uint8_t STATE : 1;  ///< the current state of the key
            uint8_t ACTIVE : 4; ///< the registered events for that key
        } bit;                  ///< bitfield format
        uint8_t reg;            ///< register format
    };

    /** keypad module edge definitions */
    enum KeypadEdge
    {
        HIGH = 0,
        LOW,
        FALLING,
        RISING,
    };

    struct Config
    {
        typename Transport::Config transport_config;
        NeoPixelI2C::Config        pixels_conf;

        Config() {}
    };

    enum Result
    {
        OK = 0,
        ERR
    };

    typedef void (*TrellisCallback)(keyEvent evt); //< Trellis Callback typedef

    /** Initialize the NeoTrellis device
        \param config Configuration settings
    */
    Result Init(Config config)
    {
        config_ = config;

        // init neopixels
        if(pixels.Init(config_.pixels_conf) == NeoPixelI2C::Result::ERR)
        {
            return ERR;
        }

        transport_.Init(config_.transport_config);

        // 10 ms delay
        System::Delay(10);

        EnableKeypadInterrupt();

        return GetTransportError();
    }

    /**  Writes an 8 bit value
        \param reg the register address to write to
        \param value the value to write to the register
    */
    void Write8(uint8_t reg_high, uint8_t reg_low, uint8_t value)
    {
        return transport_.Write8(reg_high, reg_low, value);
    }

    /**  Reads an 8 bit value
        \param reg the register address to read from
        \returns the data uint8_t read from the device
    */
    uint8_t Read8(uint8_t reg_high, uint8_t reg_low, int delay)
    {
        return transport_.Read8(reg_high, reg_low, delay);
    }

    void ReadLen(uint8_t  reg_high,
                 uint8_t  reg_low,
                 uint8_t *buff,
                 uint8_t  len,
                 int      delay)
    {
        transport_.ReadLen(reg_high, reg_low, buff, len, delay);
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
        return Write8(SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST, 0xFF);
    }

    /** activate or deactivate a given key event
        \param  key the key number to map the event to
        \param  edge the edge sensitivity of the event
        \param  enable pass true to enable the passed event, false to disable it.
    */
    void ActivateKey(uint8_t x, uint8_t y, uint8_t edge, bool enable)
    {
        int xkey = NEO_TRELLIS_X(x);
        int ykey
            = NEO_TRELLIS_Y(y % NEO_TRELLIS_NUM_ROWS * NEO_TRELLIS_NUM_COLS);

        SetKeypadEvent(
            NEO_TRELLIS_KEY(NEO_TRELLIS_XY(xkey, ykey)), edge, enable);
    }

    /** read all events currently stored in the seesaw fifo and call any callbacks.  
        \param polling pass true if the interrupt pin is not being used, false if
        it is. Defaults to true. 
    */
    void Process(bool polling = true)
    {
        uint8_t count = GetKeypadCount();
        System::DelayUs(500);
        if(count > 0)
        {
            if(polling)
                count = count + 2;
            keyEventRaw e[count];
            ReadKeypad(e, count);
            for(int i = 0; i < count; i++)
            {
                e[i].bit.NUM = NEO_TRELLIS_SEESAW_KEY(e[i].bit.NUM);
                if(e[i].bit.NUM < NEO_TRELLIS_NUM_KEYS)
                {
                    keyEvent evt = {e[i].bit.EDGE, e[i].bit.NUM};

                    state_[evt.bit.NUM]
                        = evt.bit.EDGE == HIGH || evt.bit.EDGE == RISING;
                    rising_[evt.bit.NUM]  = evt.bit.EDGE == RISING;
                    falling_[evt.bit.NUM] = evt.bit.EDGE == FALLING;

                    // call any callbacks associated with the key
                    if(_callbacks[e[i].bit.NUM] != NULL)
                    {
                        _callbacks[e[i].bit.NUM](evt);
                    }
                }
            }
        }
    }

    /** Is this key currently pressed or released? Updated via the Process() function.
        \param idx Key to check
        \return True if pressed, false otherwise
    */
    bool GetState(uint8_t idx)
    {
        if(idx < NEO_TRELLIS_NUM_KEYS)
        {
            return state_[idx];
        }

        return false;
    }

    /** Has this key just been pressed? (aka rising edge) Updated via the Process() function.
        \param idx Key to check
        \return True if just pressed, false otherwise
    */
    bool GetRising(uint8_t idx)
    {
        if(idx < NEO_TRELLIS_NUM_KEYS)
        {
            bool tmp     = rising_[idx];
            rising_[idx] = false;
            return tmp;
        }

        return false;
    }

    /** Has this key just been released? (aka falling edge) Updated via the Process() function.
        \param idx Key to check
        \return True if just released, false otherwise
    */
    bool GetFalling(uint8_t idx)
    {
        if(idx < NEO_TRELLIS_NUM_KEYS)
        {
            bool tmp      = falling_[idx];
            falling_[idx] = false;
            return tmp;
        }

        return false;
    }

    void ReadKeypad(keyEventRaw *buf, uint8_t count)
    {
        ReadLen(SEESAW_KEYPAD_BASE,
                SEESAW_KEYPAD_FIFO,
                (uint8_t *)buf,
                count,
                1000);
    }

    /** Get the number of events currently in the fifo
        \return     the number of events in the fifo
    */
    uint8_t GetKeypadCount()
    {
        return Read8(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_COUNT, 500);
    }

    /** activate or deactivate a key and edge on the keypad module
        \param      key the key number to activate
        \param		edge the edge to trigger on
        \param		enable passing true will enable the passed event, passing false will disable it.
    */
    void SetKeypadEvent(uint8_t key, uint8_t edge, bool enable)
    {
        keyState ks;
        ks.bit.STATE  = enable;
        ks.bit.ACTIVE = (1 << edge);
        uint8_t cmd[] = {SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_EVENT, key, ks.reg};
        transport_.Write(cmd, 4);
    }

    /** Enable the keypad interrupt that fires when events are in the fifo. */
    void EnableKeypadInterrupt()
    {
        Write8(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_INTENSET, 0x01);
    }

    /** register a callback for a key addressed by key index.
        \param  x the column index of the key. column 0 is on the lefthand side of the matix.
        \param  y the row index of the key. row 0 is at the top of the matrix and the numbers increase downwards.
        \param  cb the function to be called when an event from the specified key is detected.
    */
    void RegisterCallback(uint8_t x, uint8_t y, TrellisCallback (*cb)(keyEvent))
    {
        int xkey = NEO_TRELLIS_X(x);
        int ykey
            = NEO_TRELLIS_Y(y % NEO_TRELLIS_NUM_ROWS * NEO_TRELLIS_NUM_COLS);

        _callbacks[NEO_TRELLIS_XY(xkey, ykey)] = cb;
    }

    /** Unregister a callback for a key addressed by key index.
        \param  x the column index of the key. column 0 is on the lefthand side of the matix.
        \param  y the row index of the key. row 0 is at the top of the matrix and the numbers increase downwards.
    */
    void UnregisterCallback(uint8_t x, uint8_t y)
    {
        int xkey = NEO_TRELLIS_X(x);
        int ykey
            = NEO_TRELLIS_Y(y % NEO_TRELLIS_NUM_ROWS * NEO_TRELLIS_NUM_COLS);

        _callbacks[NEO_TRELLIS_XY(xkey, ykey)] = NULL;
    }

    NeoPixelI2C pixels;

  private:
    Config    config_;
    Transport transport_;

    bool state_[NEO_TRELLIS_NUM_KEYS];
    bool rising_[NEO_TRELLIS_NUM_KEYS];
    bool falling_[NEO_TRELLIS_NUM_KEYS];

    TrellisCallback (*_callbacks[NEO_TRELLIS_NUM_KEYS])(
        keyEvent); ///< the array of callback functions

}; // namespace daisy

/** @} */

using NeoTrellisI2C = NeoTrellis<NeoTrellisI2CTransport>;
} // namespace daisy
#endif