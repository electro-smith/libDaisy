#pragma once
#ifndef DSY_MAX11300_H
#define DSY_MAX11300_H

#include "daisy_core.h"
#include "per/spiMultislave.h"
#include "sys/system.h"
#include <cstring>


namespace daisy
{
/** @addtogroup device
 *  @{
 */

/** @addtogroup MAX11300
 *  @{
 */

//MAX11300 register definitions
#define MAX11300_DEVICE_ID 0x00
#define MAX11300_DEVCTL 0x10
#define MAX11300_FUNC_BASE 0x20
#define MAX11300_GPIDAT 0x0b
#define MAX11300_GPODAT 0x0d
#define MAX11300_ADCDAT_BASE 0x40
#define MAX11300_DACDAT_BASE 0x60
#define MAX11300_TRANSPORT_BUFFER_LENGTH 41

namespace MAX11300Types
{
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
     * Pins of the MAX11300 configured for AnalogRead may be defined to 
     * operate within several pre-defined voltage ranges (assuming the power supply 
     * requirements for the range is met).
     * 
     * Pins configiured for DigitalRead are 0-5V only, and do not tolerate negative
     * voltages.
     * 
     * WARNING, when a pin is configured as DigitalRead and a voltage lower than
     * -250mV is applied, the codes read from ALL other pins confiured as
     * AnalogRead will become unusuably corrupted.
     */
    enum class AdcVoltageRange
    {
        ZERO_TO_10       = 0x0100,
        NEGATIVE_5_TO_5  = 0x0200,
        NEGATIVE_10_TO_0 = 0x0300,
        ZERO_TO_2P5      = 0x0400
    };

    /**
     * Pins of the MAX11300 configured for AnalogWrite may be defined to 
     * operate within several pre-defined voltage ranges (assuming the power supply 
     * requirements for the range is met).
     * 
     * Pins configiured for DigitalWrite are 0-5V only, and do not produce negative
     * voltages.
     */
    enum class DacVoltageRange
    {
        ZERO_TO_10       = 0x0100,
        NEGATIVE_5_TO_5  = 0x0200,
        NEGATIVE_10_TO_0 = 0x0300,
    };

    /**
     * Indicates the success or failure of an operation within this class
     */
    enum class Result
    {
        OK, /**< & */
        ERR /**< & */
    };

    /** a callback type used by the transport layer */
    typedef void (*TransportCallbackFunctionPtr)(void*             context,
                                                 SpiHandle::Result result);

    /** A dma buffer that the user must put in non-cached memory. */
    struct DmaBuffer
    {
        uint8_t rx_buffer[MAX11300_TRANSPORT_BUFFER_LENGTH];
        uint8_t tx_buffer[MAX11300_TRANSPORT_BUFFER_LENGTH];
    };

    /** A function called when all MAX11300s have been updated */
    typedef void (*UpdateCompleteCallbackFunctionPtr)(void* context);

} // namespace MAX11300Types

class MAX11300MultiSlaveSpiTransport
{
  public:
    /**
     * Transport configuration struct for the MAX11300
     */
    template <size_t numDevices>
    struct Config
    {
        struct PinConfig
        {
            dsy_gpio_pin nss[numDevices] = {{DSY_GPIOG, 10}}; // Pin 7
            dsy_gpio_pin mosi            = {DSY_GPIOB, 5};    // Pin 10
            dsy_gpio_pin miso            = {DSY_GPIOB, 4};    // Pin 9
            dsy_gpio_pin sclk            = {DSY_GPIOG, 11};   // Pin 8
        } pin_config;

        SpiHandle::Config::Peripheral periph
            = SpiHandle::Config::Peripheral::SPI_1;
        SpiHandle::Config::BaudPrescaler baud_prescaler
            = SpiHandle::Config::BaudPrescaler::PS_8;
    };

    enum class Result
    {
        OK, /**< & */
        ERR /**< & */
    };

    template <size_t num_devices>
    Result Init(Config<num_devices> config)
    {
        MultiSlaveSpiHandle::Config spi_config;
        spi_config.pin_config.mosi = config.pin_config.mosi;
        spi_config.pin_config.miso = config.pin_config.miso;
        spi_config.pin_config.sclk = config.pin_config.sclk;
        const auto clamped_num_devices
            = std::min(num_devices, MultiSlaveSpiHandle::max_num_devices_);
        for(size_t i = 0; i < clamped_num_devices; i++)
            spi_config.pin_config.nss[i] = config.pin_config.nss[i];
        spi_config.periph         = config.periph;
        spi_config.direction      = SpiHandle::Config::Direction::TWO_LINES;
        spi_config.datasize       = 8;
        spi_config.clock_polarity = SpiHandle::Config::ClockPolarity::LOW;
        spi_config.clock_phase    = SpiHandle::Config::ClockPhase::ONE_EDGE;
        spi_config.baud_prescaler = config.baud_prescaler;
        // not using clamped value here on purpose to escalate errors from SPI init
        spi_config.num_devices = num_devices;
        num_devices_           = num_devices;

        const auto result = spi_.Init(spi_config);

        ready_ = result == SpiHandle::Result::OK;
        return ready_ ? Result::OK : Result::ERR;
    }

    bool Ready() { return ready_; }

    Result TransmitBlocking(size_t device_index, uint8_t* buff, size_t size)
    {
        if(spi_.BlockingTransmit(device_index, buff, size)
           == SpiHandle::Result::ERR)
        {
            return Result::ERR;
        }
        return Result::OK;
    }

    Result
    TransmitDma(size_t                                      device_index,
                uint8_t*                                    buff,
                size_t                                      size,
                MAX11300Types::TransportCallbackFunctionPtr complete_callback,
                void*                                       callback_context)
    {
        if(spi_.DmaTransmit(device_index,
                            buff,
                            size,
                            nullptr, // start callback
                            complete_callback,
                            callback_context)
           == SpiHandle::Result::ERR)
        {
            return Result::ERR;
        }
        return Result::OK;
    }

    Result TransmitAndReceiveBlocking(size_t   device_index,
                                      uint8_t* tx_buff,
                                      uint8_t* rx_buff,
                                      size_t   size)
    {
        if(spi_.BlockingTransmitAndReceive(device_index, tx_buff, rx_buff, size)
           == SpiHandle::Result::ERR)
        {
            return Result::ERR;
        }

        return Result::OK;
    }

    Result TransmitAndReceiveDma(
        size_t                                      device_index,
        uint8_t*                                    tx_buff,
        uint8_t*                                    rx_buff,
        size_t                                      size,
        MAX11300Types::TransportCallbackFunctionPtr complete_callback,
        void*                                       callback_context)
    {
        if(spi_.DmaTransmitAndReceive(device_index,
                                      tx_buff,
                                      rx_buff,
                                      size,
                                      nullptr, // start callback
                                      complete_callback,
                                      callback_context)
           == SpiHandle::Result::ERR)
        {
            return Result::ERR;
        }
        return Result::OK;
    }

    size_t GetNumDevices() const { return num_devices_; }

  private:
    MultiSlaveSpiHandle spi_;
    size_t              num_devices_ = 0;
    bool                ready_       = false;
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
template <typename Transport, size_t num_devices>
class MAX11300Driver
{
  public:
    struct Config
    {
        typename Transport::template Config<num_devices> transport_config;
        void Defaults() { transport_config = decltype(transport_config){}; }
    };

    MAX11300Driver(){};
    ~MAX11300Driver(){};

    /**
     * Initialize the MAX11300 
     * 
     * This method verifies SPI connectivity, configures the chip to
     * operate within the scope of this implementation, and 
     * intitalizes all pins by default to High-Z mode.
     * 
     * \param config - The MAX11300 configuration
     * \param dma_buffer a buffer in DMA-accessible memory. 
     *                   Allocate it like this: `MAX11300DmaBuffer DMA_BUFFER_MEM_SECTION myBuffer;`
     */
    MAX11300Types::Result Init(Config                    config,
                               MAX11300Types::DmaBuffer* dma_buffer)
    {
        dma_buffer_                       = dma_buffer;
        update_complete_callback_         = nullptr;
        update_complete_callback_context_ = nullptr;
        run_                              = false;

        if(transport_.Init(config.transport_config) != Transport::Result::OK)
            return MAX11300Types::Result::ERR;

        sequencer_.Invalidate();

        for(size_t device_index = 0; device_index < transport_.GetNumDevices();
            device_index++)
        {
            // First, let's verify the SPI comms, and chip presence.  The DEVID register
            // is a fixed, read-only value we can compare against to ensure we're connected.
            if(ReadRegister(device_index, MAX11300_DEVICE_ID) != 0x0424)
            {
                return MAX11300Types::Result::ERR;
            }

            // Init routine (roughly) as per the datasheet pp. 49
            // These settings were chosen as best applicable for use in a Eurorack context.
            // Should the need for more configurability arise, this would be the spot to do it.

            // Setup the device...
            uint16_t devctl = 0x0000;
            // 1:0 ADCCTL[1:0] - ADC conversion mode selection = 11: Continuous sweep
            devctl = devctl | 0x0003;
            // 3:2 DACCTL[1:0] - DAC mode selection = 00: Sequential Update mode for DAC-configured ports.
            devctl = devctl | 0x0000;
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
            if(WriteRegister(device_index, MAX11300_DEVCTL, devctl)
               == MAX11300Types::Result::ERR)
            {
                return MAX11300Types::Result::ERR;
            }
            // Verify our configuration was written...
            if(ReadRegister(device_index, MAX11300_DEVCTL) != devctl)
            {
                return MAX11300Types::Result::ERR;
            }

            // Add a delay as recommended in the datasheet.
            DelayUs(200);

            // Set all pins to the default high impedance state...
            for(uint8_t i = 0; i <= MAX11300Types::Pin::PIN_19; i++)
            {
                PinConfig pin_cfg;
                pin_cfg.Defaults();
                devices_[device_index].pin_configurations_[i] = pin_cfg;
                SetPinConfig(device_index, static_cast<MAX11300Types::Pin>(i));
            }
        }

        return MAX11300Types::Result::OK;
    }

    MAX11300Types::Result ConfigurePinAsDigitalRead(size_t device_index,
                                                    MAX11300Types::Pin pin,
                                                    float threshold_voltage)
    {
        auto& device = devices_[device_index];

        if(threshold_voltage > 5.0f)
            threshold_voltage = 5.0f;

        if(threshold_voltage < 0.0f)
            threshold_voltage = 0.0f;

        device.pin_configurations_[pin].Defaults();
        device.pin_configurations_[pin].mode      = PinMode::GPI;
        device.pin_configurations_[pin].threshold = threshold_voltage;

        return SetPinConfig(device_index, pin);
    }

    MAX11300Types::Result ConfigurePinAsDigitalWrite(size_t device_index,
                                                     MAX11300Types::Pin pin,
                                                     float output_voltage)
    {
        auto& device = devices_[device_index];

        if(output_voltage > 5.0f)
            output_voltage = 5.0f;

        if(output_voltage < 0.0f)
            output_voltage = 0.0f;

        device.pin_configurations_[pin].Defaults();
        device.pin_configurations_[pin].mode      = PinMode::GPO;
        device.pin_configurations_[pin].threshold = output_voltage;

        return SetPinConfig(device_index, pin);
    }

    MAX11300Types::Result
    ConfigurePinAsAnalogRead(size_t                         device_index,
                             MAX11300Types::Pin             pin,
                             MAX11300Types::AdcVoltageRange range)
    {
        auto& device = devices_[device_index];

        device.pin_configurations_[pin].Defaults();
        device.pin_configurations_[pin].mode      = PinMode::ANALOG_IN;
        device.pin_configurations_[pin].range.adc = range;

        return SetPinConfig(device_index, pin);
    }

    MAX11300Types::Result
    ConfigurePinAsAnalogWrite(size_t                         device_index,
                              MAX11300Types::Pin             pin,
                              MAX11300Types::DacVoltageRange range)
    {
        auto& device = devices_[device_index];

        device.pin_configurations_[pin].Defaults();
        device.pin_configurations_[pin].mode      = PinMode::ANALOG_OUT;
        device.pin_configurations_[pin].range.dac = range;

        return SetPinConfig(device_index, pin);
    }


    MAX11300Types::Result DisablePin(size_t device_index, Pin pin)
    {
        auto& device = devices_[device_index];
        device.pin_configurations_[pin].Defaults();
        return SetPinConfig(device_index, pin);
    }

    /**
     * Read the raw 12 bit (0-4095) value of a given ANALOG_IN (ADC) pin.
     * 
     * *note this read is local, call MAX11300::Start() to sync with the MAX11300
     * 
     * \param pin - The pin of which to read the value
     * \return - The raw, 12 bit value of the given ANALOG_IN (ADC) pin.
     */
    uint16_t ReadAnalogPinRaw(size_t device_index, MAX11300Types::Pin pin) const
    {
        auto& device = devices_[device_index];

        if(device.pin_configurations_[pin].value == nullptr)
        {
            return 0;
        }
        return __builtin_bswap16(*device.pin_configurations_[pin].value);
    }

    /**
     * Read the value of a given ADC pin in volts.
     * 
     * *note this read is local, call MAX11300::Start() to sync with the MAX11300
     * 
     * \param pin - The pin of which to read the voltage
     * \return - The value of the given ANALOG_IN (ADC) pin in volts
     */
    float ReadAnalogPinVolts(size_t device_index, MAX11300Types::Pin pin) const
    {
        auto& device = devices_[device_index];

        return MAX11300Driver::TwelveBitUintToVolts(
            ReadAnalogPinRaw(device_index, pin),
            device.pin_configurations_[pin].range.adc);
    }

    /**
     * Write a raw 12 bit (0-4095) value to a given ANALOG_OUT (DAC) pin
     * 
     * *note this write is local, call MAX11300::Start() to sync with the MAX11300
     * 
     * \param pin - The pin of which to write the value
     * \param raw_value - the 12-bit code to write to the given Pin
     */
    void WriteAnalogPinRaw(size_t             device_index,
                           MAX11300Types::Pin pin,
                           uint16_t           raw_value)
    {
        auto& device = devices_[device_index];

        if(device.pin_configurations_[pin].value != nullptr)
        {
            *device.pin_configurations_[pin].value
                = __builtin_bswap16(raw_value);
        }
    }

    /**
     * Write a voltage value, within the bounds of the configured volatge range, 
     * to a given ANALOG_OUT (DAC) pin.
     * 
     * *note this write is local, call MAX11300::Start() to sync with the MAX11300
     * 
     * \param pin - The pin of which to write the voltage
     * \param voltage - Target voltage
     */
    void WriteAnalogPinVolts(size_t             device_index,
                             MAX11300Types::Pin pin,
                             float              voltage)
    {
        auto& device = devices_[device_index];

        auto pin_config = device.pin_configurations_[pin];
        return WriteAnalogPinRaw(
            device_index,
            pin,
            MAX11300Driver::VoltsTo12BitUint(voltage, pin_config.range));
    }

    /**
     * Read the state of a GPI pin
     * 
     * *note this read is local, call MAX11300::Start() to sync with the MAX11300
     * 
     * \param pin - The pin of which to read the value
     * \return - The boolean state of the pin
     */
    bool ReadDigitalPin(size_t device_index, MAX11300Types::Pin pin) const
    {
        auto& device = devices_[device_index];

        if(pin > MAX11300Types::Pin::PIN_15)
        {
            return static_cast<bool>((device.gpi_buffer_[4] >> (pin - 16)) & 1);
        }
        else if(pin > MAX11300Types::Pin::PIN_7)
        {
            return static_cast<bool>((device.gpi_buffer_[1] >> (pin - 8)) & 1);
        }
        else
        {
            return static_cast<bool>((device.gpi_buffer_[2] >> pin) & 1);
        }
    }

    /**
     * Write a digital state to the given GPO pin
     * 
     * *note this write is local, call MAX11300::Start() to sync with the MAX11300
     * 
     * \param pin - The pin of which to write the value
     * \param value - the boolean state to write
     */
    void
    WriteDigitalPin(size_t device_index, MAX11300Types::Pin pin, bool value)
    {
        auto& device = devices_[device_index];

        // (void) pin;
        // (void) value;
        if(value)
        {
            if(pin > MAX11300Types::Pin::PIN_15)
            {
                device.gpo_buffer_[4] |= (1 << (pin - 16));
            }
            else if(pin > MAX11300Types::Pin::PIN_7)
            {
                device.gpo_buffer_[1] |= (1 << (pin - 8));
            }
            else
            {
                device.gpo_buffer_[2] |= (1 << pin);
            }
        }
        else
        {
            if(pin > MAX11300Types::Pin::PIN_15)
            {
                device.gpo_buffer_[4] &= ~(1 << (pin - 16));
            }
            else if(pin > MAX11300Types::Pin::PIN_7)
            {
                device.gpo_buffer_[1] &= ~(1 << (pin - 8));
            }
            else
            {
                device.gpo_buffer_[2] &= ~(1 << pin);
            }
        }
    }

    /**
     * Starts to update and synchronize the MAX11300 - This method does the following:
     * 
     * - Write all current ANALOG_OUT (DAC) values to all MAX11300s
     * - Read all current ANALOG_IN (ADC) values to memory
     * - Write all GPO states to all MAX11300s
     * - Read all GPI states to memory
     * - call the provided callback function when complete (from an interrupt)
     * - repeat
     * 
     * The driver can be stopped by calling Stop().
     * @see Stop()
     * 
     * \param complete_callback An optional callback function that's called after each successful update
     *                          Keep this callback function simple and fast, it's called from an interrupt.
     * \param complete_callback_context An optional context pointer provided to the complete_callback
     */
    MAX11300Types::Result
    Start(MAX11300Types::UpdateCompleteCallbackFunctionPtr complete_callback
          = nullptr,
          void* complete_callback_context = nullptr)
    {
        if(sequencer_.IsBusy() && run_)
        {
            // When the sequencer is currently busy, we can just return right away, and only
            // update the callbacks
            update_complete_callback_         = complete_callback;
            update_complete_callback_context_ = complete_callback_context;
            return MAX11300Types::Result::OK;
        }

        run_                              = true;
        update_complete_callback_         = complete_callback;
        update_complete_callback_context_ = complete_callback_context;

        sequencer_.current_device_ = 0;
        sequencer_.current_step_   = UpdateSequencer::first_step_;
        ContinueUpdate();

        return MAX11300Types::Result::OK;
    }

    /** Call this to stop the auto updating, but complete the current update. */
    void Stop() { run_ = false; }

    /**
     * A utility funtion for converting a voltage (float) value, bound to a given
     * voltage range, to the first 12 bits (0-4095) of an unsigned 16 bit integer value. 
     * 
     * \param volts the voltage to convert
     * \param range the MAX11300Types::DacVoltageRange to constrain to
     * \return the voltage as 12 bit unsigned integer
     */
    static uint16_t VoltsTo12BitUint(float                          volts,
                                     MAX11300Types::DacVoltageRange range)
    {
        float vmax    = 0;
        float vmin    = 0;
        float vscaler = 0;
        switch(range)
        {
            case MAX11300Types::DacVoltageRange::NEGATIVE_10_TO_0:
                vmin    = -10;
                vmax    = 0;
                vscaler = 4095.0f / (vmax - vmin);
                break;
            case MAX11300Types::DacVoltageRange::NEGATIVE_5_TO_5:
                vmin    = -5;
                vmax    = 5;
                vscaler = 4095.0f / (vmax - vmin);
                break;
            case MAX11300Types::DacVoltageRange::ZERO_TO_10:
                vmin    = 0;
                vmax    = 10;
                vscaler = 4095.0f / (vmax - vmin);
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
     * \param range the MAX11300Types::AdcVoltageRange to constrain to
     * \return the value as a float voltage constrained to the given voltage range
     */
    static float TwelveBitUintToVolts(uint16_t                       value,
                                      MAX11300Types::AdcVoltageRange range)
    {
        float vmax    = 0;
        float vmin    = 0;
        float vscaler = 0;
        switch(range)
        {
            case MAX11300Types::AdcVoltageRange::NEGATIVE_10_TO_0:
                vmin    = -10;
                vmax    = 0;
                vscaler = (vmax - vmin) / 4095;
                break;
            case MAX11300Types::AdcVoltageRange::NEGATIVE_5_TO_5:
                vmin    = -5;
                vmax    = 5;
                vscaler = (vmax - vmin) / 4095;
                break;
            case MAX11300Types::AdcVoltageRange::ZERO_TO_10:
                vmin    = 0;
                vmax    = 10;
                vscaler = (vmax - vmin) / 4095;
                break;
            case MAX11300Types::AdcVoltageRange::ZERO_TO_2P5:
                vmin    = 0;
                vmax    = 2.5;
                vscaler = (vmax - vmin) / 4095;
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
        PinMode mode; /**< & */
        union
        {
            MAX11300Types::AdcVoltageRange adc;
            MAX11300Types::DacVoltageRange dac;
        } range; /**< & */
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
            range.adc = MAX11300Types::AdcVoltageRange::ZERO_TO_10;
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
    MAX11300Types::Result SetPinConfig(size_t             device_index,
                                       MAX11300Types::Pin pin)
    {
        uint16_t pin_func_cfg = 0x0000;
        auto&    device       = devices_[device_index];

        if(device.pin_configurations_[pin].mode != PinMode::NONE)
        {
            // Set the pin to high impedance mode before changing (as per the datasheet).
            WriteRegister(device_index, MAX11300_FUNC_BASE + pin, 0x0000);
            // According to the datasheet, the amount of time necessary for the pin to
            // switch to high impedance mode depends on the prior configuration.
            // The worst case recommended wait time seems to be 1ms.
            DelayUs(1000);
        }

        // Apply the pin configuration
        pin_func_cfg
            = pin_func_cfg
              | static_cast<uint16_t>(device.pin_configurations_[pin].mode);

        if(device.pin_configurations_[pin].mode == PinMode::ANALOG_OUT)
        {
            pin_func_cfg |= static_cast<uint16_t>(
                device.pin_configurations_[pin].range.dac);
        }
        else if(device.pin_configurations_[pin].mode == PinMode::ANALOG_IN)
        {
            // In ADC mode we'll average 128 samples per Update
            pin_func_cfg = pin_func_cfg | 0x00e0
                           | static_cast<uint16_t>(
                               device.pin_configurations_[pin].range.adc);
        }
        else if(device.pin_configurations_[pin].mode == PinMode::GPI)
        {
            // The DAC data register for that port needs to be set to the value corresponding to the
            // intended input threshold voltage. Any input voltage above that programmed threshold is
            // reported as a logic one. The input voltage must be between 0V and 5V.
            //  It may take up to 1ms for the threshold voltage to be effective
            WriteRegister(device_index,
                          (MAX11300_DACDAT_BASE + pin),
                          MAX11300Driver::VoltsTo12BitUint(
                              device.pin_configurations_[pin].threshold,
                              device.pin_configurations_[pin].range.dac));
        }
        else if(device.pin_configurations_[pin].mode == PinMode::GPO)
        {
            // The port’s DAC data register needs to be set first. It may require up to 1ms for the
            // port to be ready to produce the desired logic one level.
            WriteRegister(device_index,
                          (MAX11300_DACDAT_BASE + pin),
                          MAX11300Driver::VoltsTo12BitUint(
                              device.pin_configurations_[pin].threshold,
                              device.pin_configurations_[pin].range.dac));
        }

        // Write the configuration now...
        if(WriteRegister(device_index, MAX11300_FUNC_BASE + pin, pin_func_cfg)
           != MAX11300Types::Result::OK)
        {
            return MAX11300Types::Result::ERR;
        }

        // Wait for 1ms as per the datasheet
        DelayUs(1000);

        // Verify our configuration was written
        if(ReadRegister(device_index, MAX11300_FUNC_BASE + pin) != pin_func_cfg)
        {
            return MAX11300Types::Result::ERR;
        }

        // Update and re-index the pin configuration now...
        UpdatePinConfig(device_index);

        return MAX11300Types::Result::OK;
    }

    /**
     * Updates all pin configurations and ensures correct pointer assignment, and addressing
     */
    MAX11300Types::Result UpdatePinConfig(size_t device_index)
    {
        // TODO
        auto& device = devices_[device_index];

        // Zero everything out...
        std::memset(device.dac_buffer_, 0, sizeof(device.dac_buffer_));
        std::memset(device.adc_buffer_, 0, sizeof(device.adc_buffer_));
        std::memset(device.gpi_buffer_, 0, sizeof(device.gpi_buffer_));
        std::memset(device.gpo_buffer_, 0, sizeof(device.gpo_buffer_));

        device.dac_pin_count_ = 0;
        device.adc_pin_count_ = 0;
        device.gpi_pin_count_ = 0;
        device.gpo_pin_count_ = 0;

        for(uint8_t i = 0; i <= MAX11300Types::Pin::PIN_19; i++)
        {
            MAX11300Types::Pin pin = static_cast<MAX11300Types::Pin>(i);

            // Always reset the value pointer first...
            device.pin_configurations_[i].value = nullptr;

            if(device.pin_configurations_[i].mode == PinMode::ANALOG_OUT)
            {
                device.dac_pin_count_++;
                if(device.dac_pin_count_ == 1)
                {
                    // If this is the first pin of this type, we need to set
                    // the initial address of the dac_buffer_ to point at this pin.
                    // The ordering of subsequent pins is known by the MAX11300.
                    device.dac_buffer_[0] = (MAX11300_DACDAT_BASE + pin) << 1;
                }
                // set the pin_config.value to a pointer at the appropriate
                // index of the dac_buffer...
                device.pin_configurations_[i].value
                    = reinterpret_cast<uint16_t*>(
                        &device.dac_buffer_[(2 * device.dac_pin_count_) - 1]);
            }
            else if(device.pin_configurations_[i].mode == PinMode::ANALOG_IN)
            {
                device.adc_pin_count_++;
                if(device.adc_pin_count_ == 1)
                {
                    // If this is the first pin of this type, we need to set
                    // the initial address of the adc_buffer_ to point at this pin.
                    // The ordering of subsequent pins is known by the MAX11300.
                    device.adc_first_adress
                        = ((MAX11300_ADCDAT_BASE + pin) << 1) | 1;
                }
                // set the pin_config.value to a pointer at the appropriate
                // index of the adc_buffer...
                device.pin_configurations_[i].value
                    = reinterpret_cast<uint16_t*>(
                        &device.adc_buffer_[(2 * device.adc_pin_count_) - 1]);
            }
            else if(device.pin_configurations_[i].mode == PinMode::GPI)
            {
                device.gpi_pin_count_++;
            }
            else if(device.pin_configurations_[i].mode == PinMode::GPO)
            {
                device.gpo_pin_count_++;
            }
        }

        return MAX11300Types::Result::OK;
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
    uint16_t ReadRegister(size_t device_index, uint8_t address)
    {
        uint16_t val = 0;
        ReadRegister(device_index, address, &val, 1);
        return val;
    }

    /**
     * Read the values, starting at the given address, from the MAX11300.
     * \param address - the register address to begin from
     * \param values - a pointer to which the values from the MAX11300 will be written 
     * \param size - the number of bytes to read
     * \return OK if the transaction was successful 
     */
    MAX11300Types::Result ReadRegister(size_t    device_index,
                                       uint8_t   address,
                                       uint16_t* values,
                                       size_t    size)
    {
        size_t  rx_length                                 = (size * 2) + 1;
        uint8_t rx_buff[MAX11300_TRANSPORT_BUFFER_LENGTH] = {};
        uint8_t tx_buff[MAX11300_TRANSPORT_BUFFER_LENGTH] = {};
        tx_buff[0]                                        = (address << 1) | 1;

        if(transport_.TransmitAndReceiveBlocking(
               device_index, tx_buff, rx_buff, rx_length)
           != Transport::Result::OK)
        {
            return MAX11300Types::Result::ERR;
        }

        size_t rx_idx = 1;
        for(size_t i = 0; i < size; i++)
        {
            values[i] = static_cast<uint16_t>((rx_buff[rx_idx] << 8)
                                              + rx_buff[rx_idx + 1]);
            rx_idx    = rx_idx + 2;
        }
        return MAX11300Types::Result::OK;
    }

    /**
     * Write a value to a single register address of the MAX11300
     * \param address - the register address to write to
     * \param value - the value to write at the given register
     * \return OK if the transaction was successful 
     */
    MAX11300Types::Result
    WriteRegister(size_t device_index, uint8_t address, uint16_t value)
    {
        return WriteRegister(device_index, address, &value, 1);
    }

    /**
     * Write the values, starting at the given address, to the MAX11300.
     * \param address - the register address to begin from
     * \param values - a pointer to which the values to be written to the MAX11300 will be read 
     * \param size - the number of bytes to written
     * \return OK if the transaction was successful 
     */
    MAX11300Types::Result WriteRegister(size_t    device_index,
                                        uint8_t   address,
                                        uint16_t* values,
                                        size_t    size)
    {
        size_t  tx_size                                   = (size * 2) + 1;
        uint8_t tx_buff[MAX11300_TRANSPORT_BUFFER_LENGTH] = {};
        tx_buff[0]                                        = (address << 1);

        size_t tx_idx = 1;
        for(size_t i = 0; i < size; i++)
        {
            tx_buff[tx_idx++] = static_cast<uint8_t>(values[i] >> 8);
            tx_buff[tx_idx++] = static_cast<uint8_t>(values[i]);
        }

        if(transport_.TransmitBlocking(device_index, tx_buff, tx_size)
           == Transport::Result::OK)
        {
            return MAX11300Types::Result::OK;
        }

        return MAX11300Types::Result::ERR;
    }


    /**
     * Read and modify the contents of a register using the given mask and value
     * \param address - the register to read from and write to
     * \param mask - the mask to use atop the read value
     * \param value - the value to apply to the read value atop the given mask
     * \return OK if the transaction was successful
     */
    MAX11300Types::Result ReadModifyWriteRegister(size_t   device_index,
                                                  uint8_t  address,
                                                  uint16_t mask,
                                                  uint16_t value)
    {
        uint16_t reg = ReadRegister(device_index, address);
        reg          = (reg & ~mask) | (uint16_t)(value);
        return WriteRegister(device_index, address, reg);
    }

    void ContinueUpdate()
    {
        // read results from the transmission that was just completed
        switch(sequencer_.current_step_)
        {
            case UpdateSequencer::Step::start:
                sequencer_.current_device_ = 0;
                sequencer_.current_step_   = UpdateSequencer::Step::updateDac;
                break;
            case UpdateSequencer::Step::updateDac:
                // nothing to read back; we only sent data
                sequencer_.current_step_ = UpdateSequencer::Step::updateAdc;
                break;
            case UpdateSequencer::Step::updateAdc:
            {
                // read back rx data
                const size_t size
                    = (devices_[sequencer_.current_device_].adc_pin_count_ * 2)
                      + 1;
                memcpy(devices_[sequencer_.current_device_].adc_buffer_,
                       dma_buffer_->rx_buffer,
                       size);
                sequencer_.current_step_ = UpdateSequencer::Step::updateGpo;
            }
            break;
            case UpdateSequencer::Step::updateGpo:
                // nothing to read back; we only sent data
                sequencer_.current_step_ = UpdateSequencer::Step::updateGpi;
                break;
            case UpdateSequencer::Step::updateGpi:
            {
                // read back rx data
                const size_t size
                    = sizeof(devices_[sequencer_.current_device_].gpi_buffer_);
                memcpy(devices_[sequencer_.current_device_].gpi_buffer_,
                       dma_buffer_->rx_buffer,
                       size);

                sequencer_.current_device_++;
                sequencer_.current_step_ = UpdateSequencer::Step::updateDac;
            }
            break;
        }

        bool done = false;
        while(!done)
        {
            if(sequencer_.current_device_ >= num_devices)
            {
                sequencer_.Invalidate();
                if(update_complete_callback_)
                    update_complete_callback_(
                        update_complete_callback_context_);
                // retrigger if not stopped
                if(run_)
                {
                    sequencer_.current_device_ = 0;
                    sequencer_.current_step_ = UpdateSequencer::Step::updateDac;
                }
                else
                    return; // all devices complete, no retriggering
            }

            auto& device = devices_[sequencer_.current_device_];

            switch(sequencer_.current_step_)
            {
                case UpdateSequencer::Step::updateDac:
                    if(device.dac_pin_count_ > 0)
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
                        size_t tx_size = (device.dac_pin_count_ * 2) + 1;
                        memcpy(dma_buffer_->tx_buffer,
                               device.dac_buffer_,
                               tx_size);
                        transport_.TransmitDma(sequencer_.current_device_,
                                               dma_buffer_->tx_buffer,
                                               tx_size,
                                               &DmaCompleteCallback,
                                               this);
                        done = true;
                    }
                    else
                    {
                        sequencer_.current_step_
                            = UpdateSequencer::Step::updateAdc;
                        done = false; // cycle again
                    }
                    break;
                case UpdateSequencer::Step::updateAdc:
                    if(device.adc_pin_count_ > 0)
                    {
                        // Reading ADC pins is a burst transaction approximately the same as the DAC transaction
                        // as described above...
                        size_t size = (device.adc_pin_count_ * 2) + 1;
                        dma_buffer_->tx_buffer[0] = device.adc_first_adress;
                        // clear the rest of the buffer (it may contain stuff from a previous transaction)
                        for(size_t i = 1; i < size; i++)
                            dma_buffer_->tx_buffer[i] = 0x00;
                        transport_.TransmitAndReceiveDma(
                            sequencer_.current_device_,
                            dma_buffer_->tx_buffer,
                            dma_buffer_->rx_buffer,
                            size,
                            &DmaCompleteCallback,
                            this);
                        done = true;
                    }
                    else
                    {
                        sequencer_.current_step_
                            = UpdateSequencer::Step::updateGpo;
                        done = false; // cycle again
                    }
                    break;
                case UpdateSequencer::Step::updateGpo:
                    if(device.gpo_pin_count_ > 0)
                    {
                        // Writing GPO pins is a single 5 byte transaction, with the first byte being the
                        // the GPO data register, and the subsequent 4 bytes containing the state of the
                        // GPO ports to be written.
                        memcpy(dma_buffer_->tx_buffer,
                               device.gpo_buffer_,
                               sizeof(device.gpo_buffer_));
                        dma_buffer_->tx_buffer[0] = (MAX11300_GPODAT << 1);
                        transport_.TransmitDma(sequencer_.current_device_,
                                               dma_buffer_->tx_buffer,
                                               sizeof(device.gpo_buffer_),
                                               &DmaCompleteCallback,
                                               this);
                        done = true;
                    }
                    else
                    {
                        sequencer_.current_step_
                            = UpdateSequencer::Step::updateGpi;
                        done = false; // cycle again
                    }
                    break;

                case UpdateSequencer::Step::updateGpi:
                    if(device.gpi_pin_count_ > 0)
                    {
                        // Reading GPI pins is a single, 5 byte, full-duplex transaction with the first
                        // and only TX byte being the GPI register.
                        dma_buffer_->tx_buffer[0] = (MAX11300_GPIDAT << 1) | 1;
                        // clear the rest of the buffer (it may contain stuff from a previous transaction)
                        for(size_t i = 1; i < sizeof(device.gpi_buffer_); i++)
                            dma_buffer_->tx_buffer[i] = 0x00;
                        transport_.TransmitAndReceiveDma(
                            sequencer_.current_device_,
                            dma_buffer_->tx_buffer,
                            dma_buffer_->rx_buffer,
                            sizeof(device.gpi_buffer_),
                            &DmaCompleteCallback,
                            this);
                        done = true;
                    }
                    else
                    {
                        sequencer_.current_step_
                            = UpdateSequencer::Step::updateDac;
                        sequencer_
                            .current_device_++; // go to next chip in sequence
                        done = false;           // cycle again
                    }
                    break;
                default: break;
            }
        }
    }

    static void DmaCompleteCallback(void* context, SpiHandle::Result result)
    {
        auto& driver = *reinterpret_cast<MAX11300Driver*>(context);
        if(result == SpiHandle::Result::OK)
        {
            driver.ContinueUpdate();
        }
        else
        {
            driver.sequencer_.Invalidate();
        }
    }

    MAX11300Types::DmaBuffer* dma_buffer_;

    struct Device
    {
        PinConfig pin_configurations_[20];
        uint8_t   dac_pin_count_;
        uint8_t   adc_pin_count_;
        uint8_t   gpi_pin_count_;
        uint8_t   gpo_pin_count_;
        uint8_t   dac_buffer_[41];
        uint8_t   adc_first_adress;
        uint8_t   adc_buffer_[41];
        uint8_t   gpi_buffer_[5];
        uint8_t   gpo_buffer_[5];
    };
    Device devices_[num_devices];

    struct UpdateSequencer
    {
        size_t current_device_ = 0;
        enum class Step
        {
            start = 0,
            updateDac,
            updateAdc,
            updateGpo,
            updateGpi,
        };
        static constexpr auto first_step_   = Step::start;
        static constexpr auto last_step_    = Step::updateGpi;
        Step                  current_step_ = first_step_;
        bool IsBusy() const { return current_device_ < num_devices; }
        void Invalidate()
        {
            current_device_ = num_devices;
            current_step_   = first_step_;
        }
    } sequencer_;

    Transport transport_;

    MAX11300Types::UpdateCompleteCallbackFunctionPtr update_complete_callback_;
    void* update_complete_callback_context_;
    bool  run_;
};
template <size_t num_devices = 1>
using MAX11300
    = daisy::MAX11300Driver<MAX11300MultiSlaveSpiTransport, num_devices>;

/** @} */
/** @} */

}; // namespace daisy

#endif
