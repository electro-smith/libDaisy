#pragma once
#ifndef DSY_TCA9534_H
#define DSY_TCA9534_H

#include "per/gpio.h"
#include "per/i2c.h"

namespace daisy
{
/** @addtogroup external
    @{
*/

/**
 * Registers for the TCA9534 8-bit I2C GPIO expander.
 * See TI datasheet SCPS208 (TCA9534).
 *
 * Note: unlike the MCP23017, the TCA9534 has **no** internal pull-ups.
 * Use external resistors on input pins.
 */
enum class Tca9534Register : uint8_t
{
    INPUT    = 0x00, /**< Input port */
    OUTPUT   = 0x01, /**< Output port */
    POLARITY = 0x02, /**< Polarity inversion */
    CONFIG   = 0x03, /**< Configuration (1 = input, 0 = output) */
};

/** Pin direction for a single TCA9534 pin. */
enum class Tca9534Mode : uint8_t
{
    INPUT,
    OUTPUT,
};

/**
 * Driver for the Texas Instruments TCA9534 (and TCA9534A) 8-bit I2C GPIO expander.
 *
 * Address range is typically 0x20–0x27 depending on A0/A1/A2 (TCA9534A uses 0x38–0x3F).
 *
 * Two init styles are supported:
 * - Owned bus: `Init(Config)` creates/initializes an I2C peripheral (like Mcp23017).
 * - Shared bus: `Init(I2CHandle&, address)` reuses an already-configured I2CHandle
 *   (useful when LEDs / codecs already own the same bus).
 *
 * Usage (shared bus):
 * \code
 * I2CHandle i2c;
 * i2c.Init(...);
 * Tca9534 exp;
 * exp.Init(i2c, 0x21);
 * exp.SetConfig(0xFE);          // P0 output, P1–P7 inputs
 * exp.WritePin(0, true);
 * uint8_t port = 0;
 * exp.ReadInput(port);
 * \endcode
 */
class Tca9534
{
  public:
    struct Config
    {
        I2CHandle::Config i2c_config;
        uint8_t           i2c_address;

        void Defaults()
        {
            i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_1;
            i2c_config.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
            i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
            i2c_config.pin_config.scl = Pin(PORTB, 8);
            i2c_config.pin_config.sda = Pin(PORTB, 9);
            // TCA9534 default (A2/A1/A0 = 000). Do not left-shift: libDaisy
            // Read/WriteDataAtAddress expect the 7-bit address.
            i2c_address = 0x20;
        }
    };

    Tca9534()
    : i2c_(nullptr),
      address_(0x20),
      output_state_(0x00),
      config_(0xFF),
      last_ok_(false),
      owns_i2c_(false)
    {
    }

    /** Initialize with default Seed I2C1 pins and address 0x20. */
    void Init()
    {
        Config cfg;
        cfg.Defaults();
        Init(cfg);
    }

    /** Initialize and own a dedicated I2C peripheral. */
    void Init(const Config& config)
    {
        address_  = config.i2c_address;
        owns_i2c_ = true;
        owned_i2c_.Init(config.i2c_config);
        i2c_ = &owned_i2c_;
        // Datasheet power-on: all pins inputs (0xFF), outputs low.
        config_       = 0xFF;
        output_state_ = 0x00;
        WriteReg(Tca9534Register::CONFIG, config_);
        WriteReg(Tca9534Register::OUTPUT, output_state_);
    }

    /**
     * Attach to an already-initialized I2CHandle (shared bus).
     * Does not re-init the peripheral.
     */
    void Init(I2CHandle& i2c, uint8_t address = 0x20)
    {
        i2c_          = &i2c;
        address_      = address;
        owns_i2c_     = false;
        config_       = 0xFF;
        output_state_ = 0x00;
    }

    /** 1 = input, 0 = output for each bit. */
    bool SetConfig(uint8_t config)
    {
        config_ = config;
        return WriteReg(Tca9534Register::CONFIG, config_);
    }

    bool SetOutputState(uint8_t output)
    {
        output_state_ = output;
        return WriteReg(Tca9534Register::OUTPUT, output_state_);
    }

    bool PinMode(uint8_t pin, Tca9534Mode mode)
    {
        if(pin > 7)
            return false;
        if(mode == Tca9534Mode::INPUT)
            config_ |= static_cast<uint8_t>(1u << pin);
        else
            config_ &= static_cast<uint8_t>(~(1u << pin));
        return WriteReg(Tca9534Register::CONFIG, config_);
    }

    bool WritePin(uint8_t pin, bool high)
    {
        if(pin > 7)
            return false;
        if(high)
            output_state_ |= static_cast<uint8_t>(1u << pin);
        else
            output_state_ &= static_cast<uint8_t>(~(1u << pin));
        return WriteReg(Tca9534Register::OUTPUT, output_state_);
    }

    bool ReadInput(uint8_t& value)
    {
        last_ok_ = ReadReg(Tca9534Register::INPUT, value);
        return last_ok_;
    }

    bool ReadPin(uint8_t pin, bool& high)
    {
        uint8_t value = 0;
        if(!ReadInput(value) || pin > 7)
            return false;
        high = (value & static_cast<uint8_t>(1u << pin)) != 0;
        return true;
    }

    bool LastTransferOk() const { return last_ok_; }
    /** Alias matching early Cosmolab naming. */
    bool LastReadOk() const { return last_ok_; }

    uint8_t OutputState() const { return output_state_; }
    uint8_t ConfigRegister() const { return config_; }
    uint8_t Address() const { return address_; }

  private:
    bool WriteReg(Tca9534Register reg, uint8_t value)
    {
        if(i2c_ == nullptr)
        {
            last_ok_ = false;
            return false;
        }
        last_ok_ = i2c_->WriteDataAtAddress(
                       address_, static_cast<uint8_t>(reg), 1, &value, 1, 10)
                   == I2CHandle::Result::OK;
        return last_ok_;
    }

    bool ReadReg(Tca9534Register reg, uint8_t& value)
    {
        if(i2c_ == nullptr)
        {
            last_ok_ = false;
            return false;
        }
        last_ok_ = i2c_->ReadDataAtAddress(
                       address_, static_cast<uint8_t>(reg), 1, &value, 1, 10)
                   == I2CHandle::Result::OK;
        return last_ok_;
    }

    I2CHandle* i2c_;
    I2CHandle  owned_i2c_;
    uint8_t    address_;
    uint8_t    output_state_;
    uint8_t    config_;
    bool       last_ok_;
    bool       owns_i2c_;
};

/** @} */

} // namespace daisy

#endif // DSY_TCA9534_H
