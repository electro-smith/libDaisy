#pragma once
#ifndef DSY_SAI_H
#define DSY_SAI_H

#include "daisy_core.h"

namespace daisy
{
/** 
 * Support for I2S Audio Protocol with different bit-depth, samplerate options
 * Allows for master or slave, as well as freedom of selecting direction, 
 * and other behavior for each peripheral, and block.
 * 
 * DMA Transfer commands must use buffers located within non-cached memory or use cache maintenance
 * To declare an unitialized global element in the DMA memory section:
 *    int32_t DSY_DMA_BUFFER_SECTOR my_buffer[96];
 *
 * Callback functions will be called once per half of the buffer. In the above example, 
 * the callback function would be called once for every 48 samples.
 * 
 * Use SAI Handle like this:
 * 
 *  SaiHandle::Config sai_config;
 *  sai_config.periph          = SaiHandle::Config::Peripheral::SAI_1;
 *  sai_config.sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
 *  sai_config.bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
 *  sai_config.a_sync          = SaiHandle::Config::Sync::MASTER;
 *  sai_config.b_sync          = SaiHandle::Config::Sync::SLAVE;
 *  sai_config.a_dir           = SaiHandle::Config::Direction::RECEIVE;
 *  sai_config.b_dir           = SaiHandle::Config::Direction::TRANSMIT;
 *  sai_config.pin_config.fs   = {DSY_GPIOE, 4};
 *  sai_config.pin_config.mclk = {DSY_GPIOE, 2};
 *  sai_config.pin_config.sck  = {DSY_GPIOE, 5};
 *  sai_config.pin_config.sa   = {DSY_GPIOE, 6};
 *  sai_config.pin_config.sb   = {DSY_GPIOE, 3};
 *  // Then Initialize
 *  SaiHandle sai;
 *  sai.Init(sai_config);
 *  // Now you can use it:
 *  sai.StartDma(. . .);
 */
class SaiHandle
{
  public:
    /** Contains settings for initialising an SAI Interface */
    struct Config
    {
        /** Specifies the internal peripheral to use (mapped to different hardware pins) */
        enum class Peripheral
        {
            SAI_1,
            SAI_2,
        };

        /** Rate at which samples will be streaming to/from the device. */
        enum class SampleRate
        {
            SAI_8KHZ,
            SAI_16KHZ,
            SAI_32KHZ,
            SAI_48KHZ,
            SAI_96KHZ,
        };

        /** Bit Depth that the hardware expects to be transferred to/from the device. */
        enum class BitDepth
        {
            SAI_16BIT,
            SAI_24BIT,
            SAI_32BIT,
        };

        /** Specifies whether a particular block is the master or the slave 
         ** If both are set to slave, no MCLK signal will be used, and it is
         ** expected that the codec will have its own xtal.
         */
        enum class Sync
        {
            MASTER,
            SLAVE,
        };

        /** Specifies the direction for each peripheral block. */
        enum class Direction
        {
            TRANSMIT,
            RECEIVE,
        };

        Peripheral periph;
        struct
        {
            dsy_gpio_pin mclk, fs, sck, sa, sb;
        } pin_config;
        SampleRate sr;
        BitDepth   bit_depth;
        Sync       a_sync, b_sync;
        Direction  a_dir, b_dir;
    };

    /** Return values for SAI functions */
    enum class Result
    {
        OK,
        ERR,
    };

    SaiHandle() : pimpl_(nullptr) {}
    SaiHandle(const SaiHandle& other) = default;
    SaiHandle& operator=(const SaiHandle& other) = default;

    /** Initializes an SAI peripheral */
    Result Init(const Config& config);

    /** Deinitializes an SAI peripheral */
    Result DeInit();

    /** Returns the current configuration */
    const Config& GetConfig() const;

    /** Callback Function to be called when DMA transfer is complete and half complete. 
     ** This callback is prepared however the data is transmitted/received from the device.
     ** For example, using an AK4556 the data will be interleaved 24bit MSB Justified
     ** 
     ** The hid/audio class will be allow for type conversions, de-interleaving, etc.
     */
    typedef void (*CallbackFunctionPtr)(int32_t* in, int32_t* out, size_t size);

    /** Starts Rx and Tx in Circular Buffer Mode 
     ** The callback will be called when half of the buffer is ready, 
     ** and will handle size/2 samples per callback.
     */
    Result StartDma(int32_t*            buffer_rx,
                    int32_t*            buffer_tx,
                    size_t              size,
                    CallbackFunctionPtr callback);

    /** Stops the DMA stream for the SAI blocks in use. */
    Result StopDma();

    /** Returns the samplerate based on the current configuration */
    float GetSampleRate();

    /** Returns the number of samples per audio block 
     ** Calculated as Buffer Size / 2 / number of channels */
    size_t GetBlockSize();

    /** Returns the Block Rate of the current stream based on the size 
     ** of the buffer passed in, and the current samplerate. 
     */
    float GetBlockRate();

    /** Returns the current offset within the SAI buffer, will be either 0 or size/2 */
    size_t GetOffset() const;

    inline bool IsInitialized() const
    {
        return pimpl_ == nullptr ? false : true;
    }

    class Impl; /**< Private Implementation class */

  private:
    Impl* pimpl_; /**< Private Implementation */
};

} // namespace daisy

#endif
