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

    /** Initializes the UART Peripheral */
    Result Init(const Config& config);

    /** Returns the current config. */
    const Config& GetConfig() const;

    /** Reads the amount of bytes in blocking mode with a 10ms timeout.
    \param *buff Buffer  to read to
    \param size Buff size
    \param timeout How long to timeout for (10ms?)
    \return Data received
     */
    int PollReceive(uint8_t* buff, size_t size, uint32_t timeout);

    /** Starts a DMA Receive callback to fill a buffer of specified size.
    Data is populated into a FIFO queue, and can be queried with the
    functions below.
    Size of the buffer is internally fixed to 256.
    Variable message lengths are transferred to the FIFO queue 
    anytime there is 1 byte-period without incoming data
    \return OK or ERROR
    */
    Result StartRx();

    /** \return whether Rx DMA is listening or not. */
    bool RxActive();

    /** Flushes the Receive Queue
    \return OK or ERROR
    */
    Result FlushRx();

    /** Sends an amount of data in blocking mode.
    \param *buff Buffer of data to send
    \param size Buffer size
    \return OK or ERROR
     */
    Result PollTx(uint8_t* buff, size_t size);

    /** Pops the oldest byte from the FIFO. 
    \return Popped byte
     */
    uint8_t PopRx();

    /** Checks if there are any unread bytes in the FIFO
    \return 1 or 0 ??
     */
    size_t Readable();

    /** \return the result of HAL_UART_GetError() to the user. */
    int CheckError();

    class Impl; /**< & */

  private:
    Impl* pimpl_;
};

/** @} */
} // namespace daisy

#endif
