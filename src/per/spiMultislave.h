#pragma once
#ifndef DSY_SPI_MULTISLAVE_H
#define DSY_SPI_MULTISLAVE_H

#include "daisy_core.h"
#include "spi.h"
#include "gpio.h"

namespace daisy
{
/** @addtogroup serial
@{
*/

/**  
 * Handler for a serial peripheral interface that connects to multiple devices on one bus
 * such that up to 4 devices can share the same MOSI, MISO and SCLK pins.
 * Each device has its own NSS/CS pin which is software driven by the MultiSlaveSpiHandle. 
 */
class MultiSlaveSpiHandle
{
  public:
    static constexpr size_t max_num_devices_ = 4;
    struct Config
    {
        struct
        {
            dsy_gpio_pin sclk; /**< & */
            dsy_gpio_pin miso; /**< & */
            dsy_gpio_pin mosi; /**< & */
            dsy_gpio_pin nss[max_num_devices_];
        } pin_config;

        SpiHandle::Config::Peripheral    periph;
        SpiHandle::Config::Direction     direction;
        unsigned long                    datasize;
        SpiHandle::Config::ClockPolarity clock_polarity;
        SpiHandle::Config::ClockPhase    clock_phase;
        SpiHandle::Config::BaudPrescaler baud_prescaler;
        size_t                           num_devices;
    };

    MultiSlaveSpiHandle() {}
    MultiSlaveSpiHandle(const MultiSlaveSpiHandle& other) = delete;

    /** Initializes handler */
    SpiHandle::Result Init(const Config& config);

    /** Returns the current config. */
    const Config& GetConfig() const { return config_; }

    /** Blocking transmit 
     * \param device_index the index of the device
     * \param buff input buffer
     * \param size  buffer size
     * \param timeout timeout time in ms
     */
    SpiHandle::Result BlockingTransmit(size_t   device_index,
                                       uint8_t* buff,
                                       size_t   size,
                                       uint32_t timeout = 100);

    /** Polling Receive
     * \param device_index the index of the device
     * \param buff  input buffer
     * \param size  buffer size
     * \param timeout timeout time in ms
     * \return Whether the receive was successful or not
     */
    SpiHandle::Result BlockingReceive(size_t   device_index,
                                      uint8_t* buff,
                                      uint16_t size,
                                      uint32_t timeout = 100);

    /** Blocking transmit and receive
     * \param device_index the index of the device
     * \param tx_buff the transmit buffer
     * \param rx_buff the receive buffer
     * \param size the length of the transaction
     * \param timeout timeout time in ms
     */
    SpiHandle::Result BlockingTransmitAndReceive(size_t   device_index,
                                                 uint8_t* tx_buff,
                                                 uint8_t* rx_buff,
                                                 size_t   size,
                                                 uint32_t timeout = 100);

    /** DMA-based transmit 
     * \param device_index  the index of the device
     * \param buff          transmit buffer
     * \param size          buffer size
     * \param start_callback   A callback to execute when the transfer starts, or NULL.
     * \param end_callback     A callback to execute when the transfer finishes, or NULL.
     * \param callback_context A pointer that will be passed back to you in the callbacks.     
     * \return Whether the transmit was successful or not
     */
    SpiHandle::Result
    DmaTransmit(size_t                              device_index,
                uint8_t*                            buff,
                size_t                              size,
                SpiHandle::StartCallbackFunctionPtr start_callback,
                SpiHandle::EndCallbackFunctionPtr   end_callback,
                void*                               callback_context);

    /** DMA-based receive 
     * \param device_index  the index of the device
     * \param buff          receive buffer
     * \param size          buffer size
     * \param start_callback   A callback to execute when the transfer starts, or NULL.
     * \param end_callback     A callback to execute when the transfer finishes, or NULL.
     * \param callback_context A pointer that will be passed back to you in the callbacks.    
     * \return Whether the receive was successful or not
     */
    SpiHandle::Result
    DmaReceive(size_t                              device_index,
               uint8_t*                            buff,
               size_t                              size,
               SpiHandle::StartCallbackFunctionPtr start_callback,
               SpiHandle::EndCallbackFunctionPtr   end_callback,
               void*                               callback_context);

    /** DMA-based transmit and receive 
     * \param device_index the index of the device
     * \param tx_buff      the transmit buffer
     * \param rx_buff      the receive buffer
     * \param size         buffer size
     * \param start_callback   A callback to execute when the transfer starts, or NULL.
     * \param end_callback     A callback to execute when the transfer finishes, or NULL.
     * \param callback_context A pointer that will be passed back to you in the callbacks.    
     * \return Whether the receive was successful or not
     */
    SpiHandle::Result
    DmaTransmitAndReceive(size_t                              device_index,
                          uint8_t*                            tx_buff,
                          uint8_t*                            rx_buff,
                          size_t                              size,
                          SpiHandle::StartCallbackFunctionPtr start_callback,
                          SpiHandle::EndCallbackFunctionPtr   end_callback,
                          void*                               callback_context);

    /** \return the result of HAL_SPI_GetError() to the user. */
    int CheckError();

  private:
    MultiSlaveSpiHandle& operator=(const MultiSlaveSpiHandle&)
    {
        return *this;
    };

    void        EnableDevice(size_t device_index);
    void        DisableDevice(size_t device_index);
    static void DmaStartCallback(void* context);
    static void DmaEndCallback(void* context, SpiHandle::Result result);

    Config    config_;
    SpiHandle spiHandle_;
    dsy_gpio  nss_pins[max_num_devices_];

    struct DmaTransfer
    {
        int8_t                              device_index;
        SpiHandle::StartCallbackFunctionPtr start_callback;
        SpiHandle::EndCallbackFunctionPtr   end_callback;
        void*                               callback_context;

        void Invalidate() { device_index = -1; }
        bool IsValid() const { return device_index >= 0; }
    } current_dma_transfer_;
};

/** @} */
} // namespace daisy

#endif // ifndef DSY_SPI_MULTISLAVE_H
