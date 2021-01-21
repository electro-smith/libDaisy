/* TODO
- Add Interrupt/DMA modes for block transfers, etc.
- Add configuration for Buffer state, etc.
- Add independent settings (bitdepth, etc.) for each channel
*/
//
#pragma once
#ifndef DSY_DAC_H
#define DSY_DAC_H

#include "daisy_core.h"

namespace daisy
{
/** DAC handle for Built-in DAC Peripheral 
 ** 
 ** For now only Normal Mode is supported,
 ** Sample and hold mode provides reduced power consumption, but requires a bit more setup.
 **
 ** For now connecting the DAC through other internal periphearls is also not supported.
 **
 ** Since the DAC channels have dedicated pins we don't need to pass in a pin config like with
 ** other modules. However, it is still important to not try to use the DAC pins for anything else.
 ** DAC Channel 1 is on PA4, and DAC Channel 2 is on PA5
 ***/
class DacHandle
{
  public:
    /** Return Values for the DacHandle class */
    enum class Result
    {
        OK,
        ERR,
    };

    /** Configuration structure for initializing the DAC structure. */
    struct Config
    {
        /** Selects which channel(s) will be configured for use. */
        enum class Channel
        {
            ONE,
            TWO,
            BOTH,
        };

        /** Sets the Mode for the DAC channels.
         **
         ** Polling mode uses the blocking mode to transmit a single value at a time.
         **
         ** DMA mode uses a buffer, and periodically transmits it triggering a 
         ** callback to fill the buffer when it is ready for more samples.
         ***/
        enum class Mode
        {
            POLLING,
            DMA,
        };

        /** Sets the number of bits per sample transmitted out of the DAC.
         ** The output range will be: 0V - VDDA
         ** The resolution will be roughly: bitdepth / (VDDA - 0V) 
         ***/
        enum class BitDepth
        {
            BITS_8,
            BITS_12
        };

        /** Sets whether the DAC output is buffered for higher drive ability. */
        enum class BufferState
        {
            ENABLED,
            DISABLED,
        };

        Channel     chn;
        Mode        mode;
        BitDepth    bitdepth;
        BufferState buff_state;
    };

    DacHandle() : pimpl_(nullptr) {}
    DacHandle(const DacHandle &other) = default;
    DacHandle &operator=(const DacHandle &other) = default;

    typedef void (*DacCallback)(uint16_t *out, size_t size);

    Result        Init(const Config &config);
    const Config &GetConfig() const;

    /** Starts the DAC conversion on the DMA calling the user callback 
     ** whenever new samples are ready to be filled. 
     ** If Channel is set to BOTH, and this function is used. the same data will 
     ** be transmitted to both channels.
     */
    Result Start(uint16_t *buffer, size_t size, DacCallback cb);

    /** If using both channels, use this function to start the DMA transfer for both. */
    Result
    Start(uint16_t *buffer_1, uint16_t *buffer_2, size_t size, DacCallback cb);

    /** Stops the DAC channel(s). */
    Result Stop();

    /** Sets and Writes value in Polling Mode 
     ** Has no effect in DMA mode.*/
    Result WriteValue(Config::Channel chn, uint16_t val);

    /** Private Implementation class */
    class Impl;

  private:
    Impl *pimpl_;
};
} // namespace daisy

// Old Code

#ifdef __cplusplus
extern "C"
{
#endif

    /** @addtogroup analog_digital_conversion
    @{
    */


    /** Driver for the built in DAC on the STM32
    The STM32 has 2 Channels of independently configurable
    DACs, with up to 12-bit resolution.
    */

    /** Currently only Polling is supported. */
    typedef enum
    {
        DSY_DAC_MODE_POLLING, /**< Polling mode */
        DSY_DAC_MODE_LAST,    /**< 3 */
    } dsy_dac_mode;

    /** 
      Sets the bit depth of the DAC output
      This can be set independently for each channel.
  */
    typedef enum
    {
        DSY_DAC_BITS_8,    /**< & */
        DSY_DAC_BITS_12,   /**< & */
        DSY_DAC_BITS_LAST, /**< & */
    } dsy_dac_bitdepth;

    /** Sets which channel(s) are initialized with the settings chosen. */
    typedef enum
    {
        DSY_DAC_CHN1,     /**< & */
        DSY_DAC_CHN2,     /**< & */
        DSY_DAC_CHN_LAST, /**< & */
        DSY_DAC_CHN_BOTH, /**< & */
    } dsy_dac_channel;

    /** 
    Configuration structure for DAC initialization and settings.
    pin_config must be filled out. However, the DACs are pretty
    consistently on pins PA4, and PA5 across all STM32 MCUs
    that I've used.
    */
    typedef struct
    {
        dsy_dac_mode     mode;                         /**< & */
        dsy_dac_bitdepth bitdepth;                     /**< & */
        dsy_gpio_pin     pin_config[DSY_DAC_CHN_LAST]; /**< & */
    } dsy_dac_handle;

    /** Initializes the specified channel(s) of the DAC 
    \param *dsy_hdac Dac to initialize 
    \param channel Channels to init
     */
    void dsy_dac_init(dsy_dac_handle *dsy_hdac, dsy_dac_channel channel);

    /** Turns on the DAC and turns on any internal timer if necessary. 
    \param channel Channel to start
    */
    void dsy_dac_start(dsy_dac_channel channel);

    /** Sets the specified channel of the dac to the value (within bitdepth) resolution.
    When set to 8-bit, val should be 0-255
    When set to 12-bit, val should be 0-4095
    \param channel Channel to write to
    \param val Value to write
    */
    void dsy_dac_write(dsy_dac_channel channel, uint16_t val);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
