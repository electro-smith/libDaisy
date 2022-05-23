/* 
TODO
- UART1 defaults to DMA, add flexible config for DMA on all periphs
- Transmit function improvements.
- Overflow handling, etc. for Rx Queue.
*/

#pragma once
#ifndef DSY_UART_H
#define DSY_UART_H /**< macro */
#include "daisy_core.h"


namespace daisy
{
/** @addtogroup serial
    @{
    */

/** 
    Uart Peripheral
    @author shensley
    @date March 2020
    */
class UartHandler
{
  public:
    struct Config
    {
        enum class Peripheral
        {
            USART_1,
            USART_2,
            USART_3,
            UART_4,
            UART_5,
            USART_6,
            UART_7,
            UART_8,
            LPUART_1,
        };

        enum class StopBits
        {
            BITS_0_5,
            BITS_1,
            BITS_1_5,
            BITS_2,
        };

        enum class Parity
        {
            NONE,
            EVEN,
            ODD,
        };

        enum class Mode
        {
            RX,
            TX,
            TX_RX,
        };

        enum class WordLength
        {
            BITS_7,
            BITS_8,
            BITS_9,
        };

        struct
        {
            dsy_gpio_pin tx; /**< & */
            dsy_gpio_pin rx; /**< & */
        } pin_config;        /**< & */

        Config()
        {
            // user must init periph, pin_config, and mode
            stopbits   = StopBits::BITS_1;
            parity     = Parity::NONE;
            wordlength = WordLength::BITS_8;
            baudrate   = 4800;
            // baudrate   = 31250;
        }

        Peripheral periph;
        StopBits   stopbits;
        Parity     parity;
        Mode       mode;
        WordLength wordlength;
        uint32_t   baudrate;
    };


    UartHandler() : pimpl_(nullptr) {}
    UartHandler(const UartHandler& other) = default;
    UartHandler& operator=(const UartHandler& other) = default;

    /** Return values for Uart functions. */
    enum class Result
    {
        OK, /**< & */
        ERR /**< & */
    };

    enum class DmaDirection
    {
        RX, /**< & */
        TX  /**< & */
    };

    /** Initializes the UART Peripheral */
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
    Result
    BlockingReceive(uint8_t* buffer, uint16_t size, uint32_t timeout = 100);

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
    Result DmaTransmit(uint8_t*                              buff,
                       size_t                                size,
                       UartHandler::StartCallbackFunctionPtr start_callback,
                       UartHandler::EndCallbackFunctionPtr   end_callback,
                       void*                                 callback_context);

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
    Result DmaReceive(uint8_t*                              buff,
                      size_t                                size,
                      UartHandler::StartCallbackFunctionPtr start_callback,
                      UartHandler::EndCallbackFunctionPtr   end_callback,
                      void*                                 callback_context);

    /** \return the result of HAL_UART_GetError() to the user. */
    int CheckError();

    /** Start the DMA Receive with a double buffered FIFO
        \return OK or ERR
    */
    Result DmaReceiveFifo();

    /** Flush all of the data from the fifo
        \return OK or ERR
    */
    Result FlushFifo();

    /** Get the top item off of the FIFO
        \return Top item from the FIFO
    */
    uint8_t PopFifo();

    /** How much data is in the FIFO
        \return number of elements ready to pop from FIFO
    */
    size_t ReadableFifo();

    /** Will be deprecated soon! Wrapper for BlockingTransmit */
    int PollReceive(uint8_t* buff, size_t size, uint32_t timeout);

    /** Will be deprecated soon! Wrapper for BlockingTransmit */
    Result PollTx(uint8_t* buff, size_t size);

    /** Will be deprecated soon! Wrapper for DmaReceiveFifo */
    Result StartRx();

    /** Will be deprecated soon! 
        \return true. New DMA will always restart itself.
    */
    bool RxActive() { return true; }

    /** Will be deprecated soon! Wrapper for FlushFifo */
    Result FlushRx();

    /** Will be deprecated soon! Wrapper PopFifo */
    uint8_t PopRx();

    /** Will be deprecated soon!  Wrapper for ReadableFifo */
    size_t Readable();

    class Impl; /**< & */

  private:
    Impl* pimpl_;
};

extern "C"
{
    /** internal. Used for global init. */
    void dsy_uart_global_init();
};

/** @} */
} // namespace daisy

#endif
