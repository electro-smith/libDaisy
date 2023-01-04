/* 
TODO
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
            baudrate   = 31250;
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

    /** A callback to be executed right before a standard dma transfer is started. */
    typedef void (*StartCallbackFunctionPtr)(void* context);
    /** A callback to be executed after a standard dma transfer is completed. */
    typedef void (*EndCallbackFunctionPtr)(void* context, Result result);

    /** A callback to be executed when using circular/listening mode 
     *  includes a callback context, as well as the data to be handled
     *  This fires either after half of the size of the user-defined buffer 
     *  has been transferred from peripheral to memory, or after an IDLE frame
     *  is detected.
     * 
     *  @param data byte-buffer to fill with data
     *  @param size size of the "data" byte buffer 
     *  @param context user-defined context variable to pass state to the callback
     *  @param result state of the UART Handler result, should be OK if things are OK.
     */
    typedef void (*CircularRxCallbackFunctionPtr)(uint8_t* data,
                                                  size_t   size,
                                                  void*    context,
                                                  Result   result);

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

    /** Starts the DMA Reception in "Listen" mode. 
     *  In this mode the DMA is configured for circular 
     *  behavior, and the IDLE interrupt is enabled.
     * 
     *  At TC, HT, and IDLE interrupts data must be processed.
     * 
     *  Size must be set so that at maximum bandwidth, the software
     *  has time to process N bytes before the next circular IRQ is fired
     * 
     *  @param buff buffer of data accessible by DMA.
     *  @param size size of buffer
     *  @param cb callback that happens containing new bytes to process in software
     *  @param callback_context pointer to user-defined data accessible from callback 
     */
    Result DmaListenStart(uint8_t*                      buff,
                          size_t                        size,
                          CircularRxCallbackFunctionPtr cb,
                          void*                         callback_context);

    /** Stops the DMA Reception during listen mode */
    Result DmaListenStop();

    /** Returns whether listen the DmaListen mode is active or not */
    bool IsListening() const;

    /** \return the result of HAL_UART_GetError() to the user. */
    int CheckError();

    /** Will be deprecated soon! Wrapper for BlockingTransmit */
    int PollReceive(uint8_t* buff, size_t size, uint32_t timeout);

    /** Will be deprecated soon! Wrapper for BlockingTransmit */
    Result PollTx(uint8_t* buff, size_t size);

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
