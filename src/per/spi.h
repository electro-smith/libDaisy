#pragma once
#ifndef DSY_SPI_H
#define DSY_SPI_H

#include "daisy_core.h"

/* TODO:
- Add documentation
- Add IT
*/

namespace daisy
{
/** @addtogroup serial
@{
*/

/**  Handler for serial peripheral interface */
class SpiHandle
{
  public:
    struct Config
    {
        enum class Peripheral
        {
            SPI_1,
            SPI_2,
            SPI_3,
            SPI_4,
            SPI_5,
            SPI_6,
        };

        enum class Mode
        {
            MASTER,
            SLAVE,
        };

        enum class Direction
        {
            TWO_LINES,
            TWO_LINES_TX_ONLY,
            TWO_LINES_RX_ONLY,
            ONE_LINE,
        };

        enum class ClockPolarity
        {
            LOW,
            HIGH,
        };

        enum class ClockPhase
        {
            ONE_EDGE,
            TWO_EDGE,
        };

        enum class NSS
        {
            SOFT,
            HARD_INPUT,
            HARD_OUTPUT,
        };

        enum class BaudPrescaler
        {
            PS_2,
            PS_4,
            PS_8,
            PS_16,
            PS_32,
            PS_64,
            PS_128,
            PS_256,
        };

        struct
        {
            dsy_gpio_pin sclk; /**< & */
            dsy_gpio_pin miso; /**< & */
            dsy_gpio_pin mosi; /**< & */
            dsy_gpio_pin nss;  /**< & */
        } pin_config;

        Peripheral    periph;
        Mode          mode;
        Direction     direction;
        unsigned long datasize;
        ClockPolarity clock_polarity;
        ClockPhase    clock_phase;
        NSS           nss;
        BaudPrescaler baud_prescaler;
    };

    SpiHandle() : pimpl_(nullptr) {}
    SpiHandle(const SpiHandle& other) = default;
    SpiHandle& operator=(const SpiHandle& other) = default;

    /** Return values for Spi functions. */
    enum class Result
    {
        OK, /**< & */
        ERR /**< & */
    };

    enum class DmaDirection
    {
        RX,    /**< & */
        TX,    /**< & */
        RX_TX, /**< & */
    };

    /** Initializes handler */
    Result Init(const Config& config);

    /** Returns the current config. */
    const Config& GetConfig() const;

    /** A callback to be executed right before a dma transfer is started. */
    typedef void (*StartCallbackFunctionPtr)(void* context);
    /** A callback to be executed after a dma transfer is completed. */
    typedef void (*EndCallbackFunctionPtr)(void* context, Result result);


    /** Blocking transmit 
    \param buff input buffer
    \param size  buffer size
    \param timeout how long in milliseconds the function will wait 
                   before returning without successful communication
    */
    Result BlockingTransmit(uint8_t* buff, size_t size, uint32_t timeout = 100);

    /** Polling Receive
    \param buffer input buffer
    \param size  buffer size
    \param timeout How long to timeout for in milliseconds
    \return Whether the receive was successful or not
    */
    Result BlockingReceive(uint8_t* buffer, uint16_t size, uint32_t timeout);

    /** Blocking transmit and receive
    \param tx_buff the transmit buffer
    \param rx_buff the receive buffer
    \param size the length of the transaction
    \param timeout how long in milliseconds the function will wait 
                   before returning without successful communication
    */
    Result BlockingTransmitAndReceive(uint8_t* tx_buff,
                                      uint8_t* rx_buff,
                                      size_t   size,
                                      uint32_t timeout = 100);

    /** DMA-based transmit 
    \param *buff input buffer
    \param size  buffer size
    \param start_callback   A callback to execute when the transfer starts, or NULL.
                            The callback is called from an interrupt, so keep it fast.
    \param end_callback     A callback to execute when the transfer finishes, or NULL.
                            The callback is called from an interrupt, so keep it fast.
    \param callback_context A pointer that will be passed back to you in the callbacks.     
    \return Whether the transmit was successful or not
    */
    Result DmaTransmit(uint8_t*                            buff,
                       size_t                              size,
                       SpiHandle::StartCallbackFunctionPtr start_callback,
                       SpiHandle::EndCallbackFunctionPtr   end_callback,
                       void*                               callback_context);

    /** DMA-based receive 
    \param *buff input buffer
    \param size  buffer size
    \param start_callback   A callback to execute when the transfer starts, or NULL.
                            The callback is called from an interrupt, so keep it fast.
    \param end_callback     A callback to execute when the transfer finishes, or NULL.
                            The callback is called from an interrupt, so keep it fast.
    \param callback_context A pointer that will be passed back to you in the callbacks.    
    \return Whether the receive was successful or not
    */
    Result DmaReceive(uint8_t*                            buff,
                      size_t                              size,
                      SpiHandle::StartCallbackFunctionPtr start_callback,
                      SpiHandle::EndCallbackFunctionPtr   end_callback,
                      void*                               callback_context);

    /** DMA-based transmit and receive 
    \param tx_buff  the transmit buffer
    \param rx_buff  the receive buffer
    \param size     buffer size
    \param start_callback   A callback to execute when the transfer starts, or NULL.
                            The callback is called from an interrupt, so keep it fast.
    \param end_callback     A callback to execute when the transfer finishes, or NULL.
                            The callback is called from an interrupt, so keep it fast.
    \param callback_context A pointer that will be passed back to you in the callbacks.    
    \return Whether the receive was successful or not
    */
    Result
    DmaTransmitAndReceive(uint8_t*                            tx_buff,
                          uint8_t*                            rx_buff,
                          size_t                              size,
                          SpiHandle::StartCallbackFunctionPtr start_callback,
                          SpiHandle::EndCallbackFunctionPtr   end_callback,
                          void*                               callback_context);

    /** \return the result of HAL_SPI_GetError() to the user. */
    int CheckError();

    class Impl; /**< SPI implementation */

  private:
    Impl* pimpl_;
};

extern "C"
{
    /** internal. Used for global init. */
    void dsy_spi_global_init();
};

/** @} */
} // namespace daisy

#endif
