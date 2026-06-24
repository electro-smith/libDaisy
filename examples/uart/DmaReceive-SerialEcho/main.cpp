/** Example demonstrating echoing messages from UART to USB serial
 *
 *  This example reads input from UART using DMA, and then
 *  echoes that input back via USB Serial.
 *
 *  Each byte is written as its own line.
 */
#include "daisy_seed.h"

using namespace daisy;

/** UART Peripheral used */
const auto kUartPeripheral = UartHandler::Config::Peripheral::USART_1;
/** Pin used for Rx */
const auto kUartRxPin = seed::D14;
/** Baud rate for UART serial */
const uint32_t kUartBaudRate = 31250;
/** size of the Rx buffer in bytes */
const size_t kUartBufferSize = 256;

/** Daisy hardware interface */
DaisySeed hardware;
/** uart buffer */
uint8_t uart_buffer[kUartBufferSize];
/** Queue of bytes to transmit to the logger */
FIFO<uint8_t, kUartBufferSize> tx_queue;

/** Callback that fires whenever an IDLE interrupt occurs,
 *  or when the buffer is more than half full
 */
void RxCallback(uint8_t*            data,
                size_t              size,
                void*               context,
                UartHandler::Result res)
{
    auto space_in_queue = tx_queue.GetCapacity() - tx_queue.GetNumElements();
    // Make sure we don't overflow (overflow bytes will be dropped)
    if(size <= space_in_queue)
    {
        // push each byte into the queue
        for(size_t i = 0; i < size; i++)
            tx_queue.PushBack(data[i]);
    }
}

int main()
{
    /** Initialize the Hardware */
    hardware.Init(true);

    /** Start the USB logging, and wait until device connects to proceed. */
    hardware.StartLog(true);

    /** UART This should match the settings of the transmitter
     *  The most commonly adjusted properties are at the top of
     *  the file as consts.
     */
    UartHandler::Config uart_config;
    UartHandler         uart;
    uart_config.baudrate      = kUartBaudRate;
    uart_config.pin_config.rx = kUartRxPin;
    uart_config.periph        = kUartPeripheral;
    uart_config.parity        = UartHandler::Config::Parity::NONE;
    uart_config.stopbits      = UartHandler::Config::StopBits::BITS_1;
    uart_config.wordlength    = UartHandler::Config::WordLength::BITS_8;
    uart_config.mode          = UartHandler::Config::Mode::RX;
    uart.Init(uart_config);

    /** Start listening for data */
    hardware.PrintLine("Starting UART Receiver");
    dsy_dma_clear_cache_for_buffer(uart_buffer, kUartBufferSize);
    uart.DmaListenStart(uart_buffer, kUartBufferSize, RxCallback, nullptr);

    while(true)
    {
        /** Restart the DMA if it stopped for any reason */
        if(!uart.IsListening())
        {
            hardware.PrintLine("Restarting UART Receiver");
            dsy_dma_clear_cache_for_buffer(uart_buffer, kUartBufferSize);
            uart.DmaListenStart(
                uart_buffer, kUartBufferSize, RxCallback, nullptr);
        }

        /** Print each byte to its own line when there are things to print. */
        while(!tx_queue.IsEmpty())
        {
            uint8_t byte = tx_queue.PopFront();
            hardware.PrintLine("%d", byte);
        }

        /** Blink the LED every ~half-second */
        hardware.SetLed((System::GetNow() & 511) < 255);
    }
}
