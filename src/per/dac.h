#pragma once
#ifndef DSY_DAC_H
#define DSY_DAC_H

#include "daisy_core.h"

namespace daisy
{
/** @brief DAC handle for Built-in DAC Peripheral 
 *  @ingroup per_analog
 ** 
 ** For now only Normal Mode is supported,
 ** Sample and hold mode provides reduced power consumption, but requires a bit more setup.
 **
 ** For now connecting the DAC through other internal peripherals is also not supported.
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

    /** Configuration structure for initializing the DAC structure. */
    struct Config
    {
        /** Target Samplerate in Hz used to configure the internal 
         ** timebase for DMA mode. Does nothing in POLLING mode.
         ** If the value is 0 at Init time this will default to 48000Hz 
         ** otherwise the driver will attempt meet the target.*/
        uint32_t target_samplerate;

        Channel     chn;
        Mode        mode;
        BitDepth    bitdepth;
        BufferState buff_state;
    };

    DacHandle() : pimpl_(nullptr) {}
    DacHandle(const DacHandle &other) = default;
    DacHandle &operator=(const DacHandle &other) = default;

    /** Callback for DMA transfers. This is called every time half 
     ** of the samples of the buffer are transmitted, and the buffer is ready
     ** to be filled again.
     ** 
     ** The data is organized in arrays per channel, for example if both channels are in use:
     ** { {ch1-0, ch1-1, ch1-2 . . . ch1-N}, {ch2-0, ch2-1, ch2-2 . . . ch2--N} }
     **
     ***/
    typedef void (*DacCallback)(uint16_t **out, size_t size);

    /** Initialize the DAC Peripheral */
    Result        Init(const Config &config);
    const Config &GetConfig() const;

    /** Starts the DAC conversion on the DMA calling the user callback 
     ** whenever new samples are ready to be filled. 
     **
     ** This will return Result::ERR if used when configured to BOTH channels.
     */
    Result Start(uint16_t *buffer, size_t size, DacCallback cb);

    /** If using both channels, use this function to start the DMA transfer for both. 
     ** The callback will provide an array per-channel to fill. */
    Result
    Start(uint16_t *buffer_1, uint16_t *buffer_2, size_t size, DacCallback cb);

    /** Stops the DAC channel(s). */
    Result Stop();

    /** Sets and Writes value in Polling Mode 
     ** Has no effect in DMA mode.*/
    Result WriteValue(Channel chn, uint16_t val);

    /** Private Implementation class */
    class Impl;

  private:
    Impl *pimpl_;
};
} // namespace daisy

#endif
