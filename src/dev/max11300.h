#pragma once
#ifndef DSY_MAX11300_H
#define DSY_MAX11300_H

#include "daisy_core.h"
#include "per/spi.h"
#include "sys/system.h"
#include <cstring>

// Some register definitions
#define MAX11300_DEVICE_ID 0x00
#define MAX11300_DEVCTL 0x10
#define MAX11300_FUNC_BASE 0x20
#define MAX11300_GPIDAT 0x0b
#define MAX11300_GPODAT 0x0d
#define MAX11300_ADCDAT_BASE 0x40
#define MAX11300_DACDAT_BASE 0x60

namespace daisy
{
/** @addtogroup dac
    @{
    */

class BlockingSpiTransport
{
  public:
    /**
     * Transport configuration struct for the MAX11300
     */
    struct Config
    {
        SpiHandle::Config spi_config; /**< & */

        /**
         * Default configuration for the MAX11300 using the SPI_1 peripheral
         * and its default pinout.
         */
        void Defaults()
        {
            spi_config.periph         = SpiHandle::Config::Peripheral::SPI_1;
            spi_config.mode           = SpiHandle::Config::Mode::MASTER;
            spi_config.direction      = SpiHandle::Config::Direction::TWO_LINES;
            spi_config.datasize       = 8;
            spi_config.clock_polarity = SpiHandle::Config::ClockPolarity::LOW;
            spi_config.clock_phase    = SpiHandle::Config::ClockPhase::ONE_EDGE;
            spi_config.nss            = SpiHandle::Config::NSS::HARD_OUTPUT;
            spi_config.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_2;
            spi_config.pin_config.nss = {DSY_GPIOG, 10};  // Pin 7
            spi_config.pin_config.sclk = {DSY_GPIOG, 11}; // Pin 8
            spi_config.pin_config.miso = {DSY_GPIOB, 4};  // Pin 9
            spi_config.pin_config.mosi = {DSY_GPIOB, 5};  // Pin 10
        }
    };

    enum class Result
    {
        OK, /**< & */
        ERR /**< & */
    };

    void Init(Config config)
    {
        spi_.Init(config.spi_config);
        next_tx_ = System::GetUs();
        ready_   = true;
    }

    bool Ready() { return ready_; }

    BlockingSpiTransport::Result
    Transmit(uint8_t* buff, size_t size, uint32_t wait_us)
    {
        if(wait_us > 0)
        {
            uint32_t ts = System::GetUs();
            // Since this is a transaction which requires a delay
            // we check if enough time has elapsed, if not, we simply
            // return "OK".
            if(ts < next_tx_)
            {
                // Check if the clock rolled over, the max wait time is 20*40us
                if((next_tx_ - ts) > (20 * 40))
                {
                    next_tx_ = ts + wait_us;
                    return Result::OK;
                }
                else
                {
                    return Result::OK;
                }
            }
        }

        if(spi_.BlockingTransmit(buff, size) == SpiHandle::Result::ERR)
        {
            return Result::ERR;
        }

        if(wait_us > 0)
        {
            // Reset next_tx timestamp
            next_tx_ = System::GetUs() + wait_us;
        }
        return Result::OK;
    }

    BlockingSpiTransport::Result
    TransmitAndReceive(uint8_t* tx_buff, uint8_t* rx_buff, size_t size)
    {
        if(spi_.BlockingTransmitAndReceive(tx_buff, rx_buff, size)
           == SpiHandle::Result::ERR)
        {
            return Result::ERR;
        }

        return Result::OK;
    }

  private:
    SpiHandle spi_;
    uint32_t  next_tx_;
    bool      ready_ = false;
};

/**
 * @brief Device Driver for the MAX11300 20 port ADC/DAC/GPIO device.
 * @author sam.braam
 * @date Oct. 2021
 * 
 * This is a highly opinionated driver implementation for the MAX11300
 * DAC/ADC/GPIO device.  
 * 
 * This implemetation has been designed for use in the context of Eurorack
 * modular systems. There are a number of features the MAX11300 offers
 * which are not exposed, as well as a number of configuration decisions 
 * that were made in order to simplify usage and improve ergonomics, 
 * even at the cost of flexibility.
*/
template <typename Transport>
class MAX11300Driver
{
  public:
    /**
    * Represents a pin/port on the MAX11300, of which there are 20.
    */
    enum Pin
    {
        PIN_0,
        PIN_1,
        PIN_2,
        PIN_3,
        PIN_4,
        PIN_5,
        PIN_6,
        PIN_7,
        PIN_8,
        PIN_9,
        PIN_10,
        PIN_11,
        PIN_12,
        PIN_13,
        PIN_14,
        PIN_15,
        PIN_16,
        PIN_17,
        PIN_18,
        PIN_19
    };

    /**
     * Pins of the MAX11300 configured for AnalogRead/Write may be defined to 
     * operate within several pre-defined voltage ranges (assuming the power supply 
     * requirements for the range is met).
     * 
     * Pins configiured for DigitalRead/Write are 0-5V only, and do not tolerate
     * or produce negative voltages.
     * 
     * WARNING, when a pin is configured as DigitalRead and a voltage lower than
     *  -250mV is applied, The codes read from ALL other pins confiured as
     * AnalogRead will become unusuably corrupted.
     */
    enum class VoltageRange
    {
        ZERO_TO_10       = 0x0100,
        NEGATIVE_5_TO_5  = 0x0200,
        NEGATIVE_10_TO_0 = 0x0300
    };


    struct Config
    {
        typename Transport::Config transport_config;
        void                       Defaults() { transport_config.Defaults(); }
    };

    /**
     * Indicates the success or failure of an operation within this class
     */
    enum class Result
    {
        OK, /**< & */
        ERR /**< & */
    };


    MAX11300Driver<Transport>(){};
    ~MAX11300Driver<Transport>(){};

    /**
     * Initialize the MAX11300 
     * 
     * This method verifies SPI connectivity, configures the chip to
     * operate within the scope of this implementation, and 
     * intitalizes all pins by default to High-Z mode.
     * 
     * \param config - The MAX11300 configuration
     */
    Result Init(Config config)
    {
        transport_.Init(config.transport_config);

        // First, let's verify the SPI comms, and chip presence.  The DEVID register
        // is a fixed, read-only value we can compare against to ensure we're connected.
        if(ReadRegister(MAX11300_DEVICE_ID) != 0x0424)
        {
            return Result::ERR;
        }

        // Init routine (roughly) as per the datasheet pp. 49
        // These settings were chosen as best applicable for use in a Eurorack context.
        // Should the need for more configurability arise, this would be the spot to do it.

        // Setup the device...
        uint16_t devctl = 0x0000;
        // 1:0 ADCCTL[1:0] - ADC conversion mode selection = 11: Continuous sweep
        devctl = devctl | 0x0003;
        // 3:2 DACCTL[1:0] - DAC mode selection = 01: Immediate Update mode for DAC-configured ports.
        devctl = devctl | 0x0004;
        // 5:4 ADCCONV[1:0] - ADC conversion rate selection = 11: ADC conversion rate of 400ksps
        devctl = devctl | 0x0030;
        // 6 DACREF - DAC voltage reference selection = 1: Internal reference voltage
        devctl = devctl | 0x0040;
        // 7 THSHDN  - Thermal shutdown enable = 1: Thermal shutdown function enabled.
        devctl = devctl | 0x0080;
        // 10:8 TMPCTL[2:0] - Temperature monitor selection = 001: Internal temperature monitor enabled
        devctl = devctl | 0x0100;
        // 11 TMPPER - Temperature conversion time control = 0 (Default)
        // 12 RS_CANCEL - Temperature sensor series resistor cancellation mode = 0 (Default)
        // 13 LPEN - Power mode selection = 0 (Default)
        // 14 BRST - Serial interface burst-mode selection = 1: Contextual address incrementing mode
        devctl = devctl | 0x4000;
        // 15 RESET - Soft reset control = 0 (Default)

        // Write the device configuration
        if(WriteRegister(MAX11300_DEVCTL, devctl) == Result::ERR)
        {
            return Result::ERR;
        }
        // Verify our configuration was written...
        if(ReadRegister(MAX11300_DEVCTL) != devctl)
        {
            return Result::ERR;
        }

        // Add a delay as recommended in the datasheet.
        DelayUs(200);

        // Set all pins to the default high impedance state...
        for(uint8_t i = 0; i <= Pin::PIN_19; i++)
        {
            PinConfig pin_cfg;
            pin_cfg.Defaults();
            pin_configurations_[i] = pin_cfg;
            SetPinConfig(static_cast<Pin>(i));
        }

        return Result::OK;
    }

    Result ConfigurePinAsDigitalRead(Pin pin, float threshold_voltage)
    {
        if(threshold_voltage > 5.0f)
            threshold_voltage = 5.0f;

        if(threshold_voltage < 0.0f)
            threshold_voltage = 0.0f;

        pin_configurations_[pin].Defaults();
        pin_configurations_[pin].mode      = PinMode::GPI;
        pin_configurations_[pin].threshold = threshold_voltage;

        return SetPinConfig(pin);
    }

    Result ConfigurePinAsDigitalWrite(Pin pin, float output_voltage)
    {
        if(output_voltage > 5.0f)
            output_voltage = 5.0f;

        if(output_voltage < 0.0f)
            output_voltage = 0.0f;

        pin_configurations_[pin].Defaults();
        pin_configurations_[pin].mode      = PinMode::GPO;
        pin_configurations_[pin].threshold = output_voltage;

        return SetPinConfig(pin);
    }

    Result ConfigurePinAsAnalogRead(Pin pin, VoltageRange range)
    {
        pin_configurations_[pin].Defaults();
        pin_configurations_[pin].mode  = PinMode::ANALOG_IN;
        pin_configurations_[pin].range = range;

        return SetPinConfig(pin);
    }

    Result ConfigurePinAsAnalogWrite(Pin pin, VoltageRange range)
    {
        pin_configurations_[pin].Defaults();
        pin_configurations_[pin].mode  = PinMode::ANALOG_OUT;
        pin_configurations_[pin].range = range;

        return SetPinConfig(pin);
    }


    Result DisablePin(Pin pin)
    {
        PinConfig pin_config = GetPinConfig(pin);
        pin_configurations_[pin].Defaults();
        SetPinConfig(pin_config);
    }

    /**
     * Read the raw 12 bit (0-4095) value of a given ANALOG_IN (ADC) pin.
     * 
     * *note this read is local, call MAX11300::Update() to sync with the MAX11300
     * 
     * \param pin - The pin of which to read the value
     * \return - The raw, 12 bit value of the given ANALOG_IN (ADC) pin.
     */
    uint16_t ReadAnalogPinRaw(Pin pin)
    {
        if(pin_configurations_[pin].value == nullptr)
        {
            return 0;
        }
        return __builtin_bswap16(*pin_configurations_[pin].value);
    }

    /**
     * Read the value of a given ADC pin in volts.
     * 
     * *note this read is local, call MAX11300::Update() to sync with the MAX11300
     * 
     * \param pin - The pin of which to read the voltage
     * \return - The value of the given ANALOG_IN (ADC) pin in volts
     */
    float ReadAnalogPinVolts(Pin pin)
    {
        return MAX11300Driver::TwelveBitUintToVolts(
            ReadAnalogPinRaw(pin), pin_configurations_[pin].range);
    }

    /**
     * Write a raw 12 bit (0-4095) value to a given ANALOG_OUT (DAC) pin
     * 
     * *note this write is local, call MAX11300::Update() to sync with the MAX11300
     * 
     * \param pin - The pin of which to write the value
     */
    void WriteAnalogPinRaw(Pin pin, uint16_t raw_value)
    {
        if(pin_configurations_[pin].value != nullptr)
        {
            *pin_configurations_[pin].value = __builtin_bswap16(raw_value);
        }
    }

    /**
     * Write a voltage value, within the bounds of the configured volatge range, 
     * to a given ANALOG_OUT (DAC) pin.
     * 
     * *note this write is local, call MAX11300::Update() to sync with the MAX11300
     * 
     * \param pin - The pin of which to write the voltage
     */
    void WriteAnalogPinVolts(Pin pin, float voltage)
    {
        PinConfig pin_config = pin_configurations_[pin];
        return WriteAnalogPinRaw(
            pin, MAX11300Driver::VoltsTo12BitUint(voltage, pin_config.range));
    }

    /**
     * Read the state of a GPI pin
     * 
     * *note this read is local, call MAX11300::Update() to sync with the MAX11300
     * 
     * \param pin - The pin of which to read the value
     * \return - The boolean state of the pin
     */
    bool ReadDigitalPin(Pin pin)
    {
        if(pin > Pin::PIN_15)
        {
            return static_cast<bool>((gpi_buffer_[4] >> (pin - 16)) & 1);
        }
        else if(pin > Pin::PIN_7)
        {
            return static_cast<bool>((gpi_buffer_[1] >> (pin - 8)) & 1);
        }
        else
        {
            return static_cast<bool>((gpi_buffer_[2] >> pin) & 1);
        }
    }

    /**
     * Write a digital state to the given GPO pin
     * 
     * *note this write is local, call MAX11300::Update() to sync with the MAX11300
     * 
     * \param pin - The pin of which to write the value
     * \param value - the boolean state to write
     */
    void WriteDigitalPin(Pin pin, bool value)
    {
        // (void) pin;
        // (void) value;
        if(value)
        {
            if(pin > Pin::PIN_15)
            {
                gpo_buffer_[4] |= (1 << (pin - 16));
            }
            else if(pin > Pin::PIN_7)
            {
                gpo_buffer_[1] |= (1 << (pin - 8));
            }
            else
            {
                gpo_buffer_[2] |= (1 << pin);
            }
        }
        else
        {
            if(pin > Pin::PIN_15)
            {
                gpo_buffer_[4] &= ~(1 << (pin - 16));
            }
            else if(pin > Pin::PIN_7)
            {
                gpo_buffer_[1] &= ~(1 << (pin - 8));
            }
            else
            {
                gpo_buffer_[2] &= ~(1 << pin);
            }
        }
    }

    /**
     * Update and synchronize the MAX11300 - This method does the following:
     * 
     * - Write all current ANALOG_OUT (DAC) values to the MAX11300
     * - Read all current ANALOG_IN (ADC) values to memory
     * - Write all GPO states to the MAX11300
     * - Read all GPI states to memory
     * 
     * TODO - Provide more info on usage location and the side-effects of blocking...
     * 
     */
    Result Update()
    {
        // Check first if were ready to TX/RX
        if(!transport_.Ready())
            return Result::OK;

        if(dac_pin_count_ > 0)
        {
            // This is a burst transaction utilizing the contextual addressing
            // scheme of the MAX11300. See the datasheet @ pp. 30
            //
            // We've prefixed the dac_buffer_ to point at the first dac pin to be written to.
            // Subsequent DAC pins are written in their absolute order (0-19) if configured as such.
            // For example:
            // [1st_dac_pin_addr],[1st_dac_pin_msb],[1st_dac_pin_lsb],[2nd_dac_pin_msb],[2nd_dac_pin_lsb]...
            //
            // The size of the transaction is determined by the number of configured dac pins,
            // plus one byte for the initial pin address.
            //
            // The datasheet recommends waiting 80us between DAC updates, in practice the appears to be
            // per configured DAC pin. Here we inform the transport to wait at least N uS before transmitting
            // again.
            size_t tx_size = (dac_pin_count_ * 2) + 1;
            if(transport_.Transmit(dac_buffer_, tx_size, (dac_pin_count_ * 40))
               != Transport::Result::OK)
            {
                return Result::ERR;
            }
        }

        if(adc_pin_count_ > 0)
        {
            // Reading ADC pins is a burst transaction approximately the same as the DAC transaction
            // as described above...
            size_t size = (adc_pin_count_ * 2) + 1;

            uint8_t tx_buff[MAX_TRANSPORT_BUFFER_LENGTH] = {};
            tx_buff[0]                                   = adc_buffer_[0];
            if(transport_.TransmitAndReceive(tx_buff, adc_buffer_, size)
               != Transport::Result::OK)
            {
                adc_buffer_[0] = tx_buff[0];
                return Result::ERR;
            }
            adc_buffer_[0] = tx_buff[0];
        }

        if(gpo_pin_count_ > 0)
        {
            // Writing GPO pins is a single 5 byte transaction, with the first byte being the
            // the GPO data register, and the subsequent 4 bytes containing the state of the
            // GPO ports to be written.
            if(transport_.Transmit(gpo_buffer_, sizeof(gpo_buffer_), 0)
               != Transport::Result::OK)
            {
                return Result::ERR;
            }
        }

        if(gpi_pin_count_ > 0)
        {
            // Reading GPI pins is a single, 5 byte, full-duplex transaction with the first
            // and only TX byte being the GPI register.
            uint8_t tx_buff[sizeof(gpi_buffer_)] = {};
            tx_buff[0]                           = gpi_buffer_[0];
            if(transport_.TransmitAndReceive(
                   tx_buff, gpi_buffer_, sizeof(gpi_buffer_))
               != Transport::Result::OK)
            {
                gpi_buffer_[0] = tx_buff[0];
                return Result::ERR;
            }
            gpi_buffer_[0] = tx_buff[0];
        }

        return Result::OK;
    }

    /**
     * A utility funtion for converting a voltage (float) value, bound to a given
     * voltage range, to the first 12 bits (0-4095) of an unsigned 16 bit integer value. 
     * 
     * \param volts the voltage to convert
     * \param range the MAX11300::VoltageRange to constrain to
     * \return the voltage as 12 bit unsigned integer
     */
    static uint16_t VoltsTo12BitUint(float volts, VoltageRange range)
    {
        float vmax    = 0;
        float vmin    = 0;
        float vscaler = 0;
        switch(range)
        {
            case VoltageRange::NEGATIVE_10_TO_0:
                vmin    = -10;
                vmax    = 0;
                vscaler = 4095.0f / (vmax - vmin);
                break;
            case VoltageRange::NEGATIVE_5_TO_5:
                vmin    = -5;
                vmax    = 5;
                vscaler = 4095.0f / (vmax - vmin);
                break;
            case VoltageRange::ZERO_TO_10:
                vmin    = 0;
                vmax    = 10;
                vscaler = 4095.0f / (vmax - vmin);
                break;
            default:
                // Nothing left to do
                return 0;
                break;
        }
        // Clamp...
        if(volts > vmax)
            volts = vmax;

        if(volts < vmin)
            volts = vmin;

        return static_cast<uint16_t>((volts - vmin) * vscaler);
    }

    /**
     * A utility funtion for converting the first 12 bits (0-4095) of an unsigned
     * 16 bit integer value, to a voltage (float) value. The voltage value is 
     * scaled and bound to the given voltage range.
     * 
     * \param value the 12 bit value to convert
     * \param range the MAX11300::VoltageRange to constrain to
     * \return the value as a float voltage constrained to the given voltage range
     */
    static float TwelveBitUintToVolts(uint16_t value, VoltageRange range)
    {
        float vmax    = 0;
        float vmin    = 0;
        float vscaler = 0;
        switch(range)
        {
            case VoltageRange::NEGATIVE_10_TO_0:
                vmin    = -10;
                vmax    = 0;
                vscaler = (vmax - vmin) / 4095;
                break;
            case VoltageRange::NEGATIVE_5_TO_5:
                vmin    = -5;
                vmax    = 5;
                vscaler = (vmax - vmin) / 4095;
                break;
            case VoltageRange::ZERO_TO_10:
                vmin    = 0;
                vmax    = 10;
                vscaler = (vmax - vmin) / 4095;
                break;
            default:
                // Nothing left to do
                return 0;
                break;
        }
        // Clamp...
        if(value > 4095)
            value = 4095;

        return (value * vscaler) + vmin;
    }

  private:
    /**
     * Pins/ports of the MAX11300 are freely configurable to function as 
     * ANALOG_IN (ADC), ANALOG_OUT (DAC), GPI, or GPO.  
     * This enum describes these modes.
     */
    enum class PinMode
    {
        NONE       = 0x0000, // Mode 0 (High impedance)
        GPI        = 0x1000, // Mode 1
        GPO        = 0x3000, // Mode 3
        ANALOG_OUT = 0x5000, // Mode 5
        ANALOG_IN  = 0x7000, // Mode 7
    };

    /**
     * The PinConfig struct holds the necessary information needed to configure a
     * pin/port of the MAX11300, as well as a pointer to the stateful pin data.  
     */
    struct PinConfig
    {
        PinMode      mode;  /**< & */
        VoltageRange range; /**< & */
        /**
         * This is a voltage value used as follows:
         * 
         *  GPI - Defines what input voltage constituates a logical 1
         *  GPO - The output voltage of the pin at logical 1
         */
        float threshold;
        /**
         * In the case of ANALOG_IN or ANALOG_OUT modes, this points to the 
         * current 12 bit value of the pin.
         */
        uint16_t* value;
        /**
         * Default pin settings - Disabled (High-Z mode)
         */
        void Defaults()
        {
            mode      = PinMode::NONE;
            range     = VoltageRange::ZERO_TO_10;
            threshold = 0.0f;
            value     = nullptr;
        }
    };

    /**
     * Apply the current configuration to the given pin
     * 
     * \param pin - The pin to configure
     * \return - OK if the configuration was successfully applied
     */
    Result SetPinConfig(Pin pin)
    {
        uint16_t pin_func_cfg = 0x0000;

        if(pin_configurations_[pin].mode != PinMode::NONE)
        {
            // Set the pin to high impedance mode before changing (as per the datasheet).
            WriteRegister(MAX11300_FUNC_BASE + pin, 0x0000);
            // According to the datasheet, the amount of time necessary for the pin to
            // switch to high impedance mode depends on the prior configuration.
            // The worst case recommended wait time seems to be 1ms.
            DelayUs(1000);
        }

        // Apply the pin configuration
        pin_func_cfg = pin_func_cfg
                       | static_cast<uint16_t>(pin_configurations_[pin].mode)
                       | static_cast<uint16_t>(pin_configurations_[pin].range);

        if(pin_configurations_[pin].mode == PinMode::ANALOG_IN)
        {
            // In ADC mode we'll average 128 samples per Update
            pin_func_cfg = pin_func_cfg | 0x00e0;
        }
        else if(pin_configurations_[pin].mode == PinMode::GPI)
        {
            // The DAC data register for that port needs to be set to the value corresponding to the
            // intended input threshold voltage. Any input voltage above that programmed threshold is
            // reported as a logic one. The input voltage must be between 0V and 5V.
            //  It may take up to 1ms for the threshold voltage to be effective
            WriteRegister((MAX11300_DACDAT_BASE + pin),
                          MAX11300Driver::VoltsTo12BitUint(
                              pin_configurations_[pin].threshold,
                              pin_configurations_[pin].range));
        }
        else if(pin_configurations_[pin].mode == PinMode::GPO)
        {
            // The port’s DAC data register needs to be set first. It may require up to 1ms for the
            // port to be ready to produce the desired logic one level.
            WriteRegister((MAX11300_DACDAT_BASE + pin),
                          MAX11300Driver::VoltsTo12BitUint(
                              pin_configurations_[pin].threshold,
                              pin_configurations_[pin].range));
        }

        // Write the configuration now...
        if(WriteRegister(MAX11300_FUNC_BASE + pin, pin_func_cfg) != Result::OK)
        {
            return Result::ERR;
        }

        // Wait for 1ms as per the datasheet
        DelayUs(1000);

        // Verify our configuration was written
        if(ReadRegister(MAX11300_FUNC_BASE + pin) != pin_func_cfg)
        {
            return Result::ERR;
        }

        // Update and re-index the pin configuration now...
        UpdatePinConfig();

        return Result::OK;
    }

    /**
     * Updates all pin configurations and ensures correct pointer assignment, and addressing
     */
    Result UpdatePinConfig()
    {
        // Zero everything out...
        std::memset(dac_buffer_, 0, sizeof(dac_buffer_));
        std::memset(adc_buffer_, 0, sizeof(adc_buffer_));
        std::memset(gpi_buffer_, 0, sizeof(gpi_buffer_));
        std::memset(gpo_buffer_, 0, sizeof(gpo_buffer_));

        dac_pin_count_ = 0;
        adc_pin_count_ = 0;
        gpi_pin_count_ = 0;
        gpo_pin_count_ = 0;

        for(uint8_t i = 0; i <= Pin::PIN_19; i++)
        {
            Pin pin = static_cast<Pin>(i);

            // Always reset the value pointer first...
            pin_configurations_[i].value = nullptr;

            if(pin_configurations_[i].mode == PinMode::ANALOG_OUT)
            {
                dac_pin_count_++;
                if(dac_pin_count_ == 1)
                {
                    // If this is the first pin of this type, we need to set
                    // the initial address of the dac_buffer_ to point at this pin.
                    // The ordering of subsequent pins is known by the MAX11300.
                    dac_buffer_[0] = (MAX11300_DACDAT_BASE + pin) << 1;
                }
                // set the pin_config.value to a pointer at the appropriate
                // index of the dac_buffer...
                pin_configurations_[i].value = reinterpret_cast<uint16_t*>(
                    &dac_buffer_[(2 * dac_pin_count_) - 1]);
            }
            else if(pin_configurations_[i].mode == PinMode::ANALOG_IN)
            {
                adc_pin_count_++;
                if(adc_pin_count_ == 1)
                {
                    // If this is the first pin of this type, we need to set
                    // the initial address of the adc_buffer_ to point at this pin.
                    // The ordering of subsequent pins is known by the MAX11300.
                    adc_buffer_[0] = ((MAX11300_ADCDAT_BASE + pin) << 1) | 1;
                }
                // set the pin_config.value to a pointer at the appropriate
                // index of the adc_buffer...
                pin_configurations_[i].value = reinterpret_cast<uint16_t*>(
                    &adc_buffer_[(2 * adc_pin_count_) - 1]);
            }
            else if(pin_configurations_[i].mode == PinMode::GPI)
            {
                gpi_pin_count_++;
                gpi_buffer_[0] = (MAX11300_GPIDAT << 1) | 1;
            }
            else if(pin_configurations_[i].mode == PinMode::GPO)
            {
                gpo_pin_count_++;
                gpo_buffer_[0] = (MAX11300_GPODAT << 1);
            }
        }

        return Result::OK;
    }

    // This is just a wrapper for System::DelayUs to allow it to be
    // excluded from the unit tests without dirtying up the code so much
    void DelayUs(uint32_t delay)
    {
        (void)delay;
#ifndef UNIT_TEST
        System::DelayUs(delay);
#endif
    }

    /**
     * Read the value of a single register address from the MAX11300
     * \param address - the register address to read
     * \return the value at the given register as returned by the MAX11300 
     */
    uint16_t ReadRegister(uint8_t address)
    {
        uint16_t val = 0;
        ReadRegister(address, &val, 1);
        return val;
    }

    /**
     * Read the values, starting at the given address, from the MAX11300.
     * \param address - the register address to begin from
     * \param values - a pointer to which the values from the MAX11300 will be written 
     * \param size - the number of bytes to read
     * \return OK if the transaction was successful 
     */
    Result ReadRegister(uint8_t address, uint16_t* values, size_t size)
    {
        size_t  rx_length                            = (size * 2) + 1;
        uint8_t rx_buff[MAX_TRANSPORT_BUFFER_LENGTH] = {};
        uint8_t tx_buff[MAX_TRANSPORT_BUFFER_LENGTH] = {};
        tx_buff[0]                                   = (address << 1) | 1;

        if(transport_.TransmitAndReceive(tx_buff, rx_buff, rx_length)
           != Transport::Result::OK)
        {
            return Result::ERR;
        }

        size_t rx_idx = 1;
        for(size_t i = 0; i < size; i++)
        {
            values[i] = static_cast<uint16_t>((rx_buff[rx_idx] << 8)
                                              + rx_buff[rx_idx + 1]);
            rx_idx    = rx_idx + 2;
        }
        return Result::OK;
    }

    /**
     * Write a value to a single register address of the MAX11300
     * \param address - the register address to write to
     * \param value - the value to write at the given register
     * \return OK if the transaction was successful 
     */
    Result WriteRegister(uint8_t address, uint16_t value)
    {
        return WriteRegister(address, &value, 1);
    }

    /**
     * Write the values, starting at the given address, to the MAX11300.
     * \param address - the register address to begin from
     * \param values - a pointer to which the values to be written to the MAX11300 will be read 
     * \param size - the number of bytes to written
     * \return OK if the transaction was successful 
     */
    Result WriteRegister(uint8_t address, uint16_t* values, size_t size)
    {
        size_t  tx_size                              = (size * 2) + 1;
        uint8_t tx_buff[MAX_TRANSPORT_BUFFER_LENGTH] = {};
        tx_buff[0]                                   = (address << 1);

        size_t tx_idx = 1;
        for(size_t i = 0; i < size; i++)
        {
            tx_buff[tx_idx++] = static_cast<uint8_t>(values[i] >> 8);
            tx_buff[tx_idx++] = static_cast<uint8_t>(values[i]);
        }

        if(transport_.Transmit(tx_buff, tx_size, 0) == Transport::Result::OK)
        {
            return Result::OK;
        }

        return Result::ERR;
    }


    /**
     * Read and modify the contents of a register using the given mask and value
     * \param address - the register to read from and write to
     * \param mask - the mask to use atop the read value
     * \param value - the value to apply to the read value atop the given mask
     * \return OK if the transaction was successful
     */
    Result
    ReadModifyWriteRegister(uint8_t address, uint16_t mask, uint16_t value)
    {
        uint16_t reg = ReadRegister(address);
        reg          = (reg & ~mask) | (uint16_t)(value);
        return WriteRegister(address, reg);
    }

    static const size_t MAX_TRANSPORT_BUFFER_LENGTH = 41;

    PinConfig pin_configurations_[20];

    uint8_t dac_pin_count_;

    uint8_t adc_pin_count_;

    uint8_t gpi_pin_count_;

    uint8_t gpo_pin_count_;

    uint8_t dac_buffer_[41];

    uint8_t adc_buffer_[41];

    uint8_t gpi_buffer_[5];

    uint8_t gpo_buffer_[5];

    Transport transport_;
};

using MAX11300 = daisy::MAX11300Driver<BlockingSpiTransport>;

}; // namespace daisy

#endif
