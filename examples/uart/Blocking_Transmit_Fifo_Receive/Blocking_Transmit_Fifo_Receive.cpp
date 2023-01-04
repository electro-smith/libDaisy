/** TODO fix / remove this example 
 *  anticipated issue that it only shows 1 byte every 100ms
 *  which may be misleading to user
 *  
 *  also, it should probably use serial print to pipe out
 *  data instead of the patch's display.
 */
#include "daisy_patch.h"

using namespace daisy;

/** Consts */
const size_t kUartBufferSize = 512;

/** Globals */
DaisyPatch  hw;
UartHandler uart;
uint8_t     uart_buffer[kUartBufferSize];
char        receive_str[kUartBufferSize];

/** Happens automatically whenever transaction completes */
void uartCallback(uint8_t*            data,
                  size_t              size,
                  void*               context,
                  UartHandler::Result res)
{
    /** Clear receive_str */
    std::fill(&receive_str[0], &receive_str[kUartBufferSize - 1], 0);
    /** Copy new data into the receive str */
    std::copy(&data[0], &data[size - 1], &receive_str[0]);
}

int main(void)
{
    // start the Daisy Patch
    hw.Init();

    // set up our UART peripheral
    UartHandler::Config uart_conf;
    uart_conf.periph        = UartHandler::Config::Peripheral::USART_1;
    uart_conf.mode          = UartHandler::Config::Mode::TX_RX;
    uart_conf.pin_config.tx = Pin(PORTB, 6);
    uart_conf.pin_config.rx = Pin(PORTB, 7);

    // initialize the UART peripheral, and start reading
    uart.Init(uart_conf);
    uart.DmaListenStart(uart_buffer, kUartBufferSize, uartCallback, nullptr);

    uint8_t pop  = 0;
    uint8_t send = 0;
    while(1)
    {
        // send the data in a blocking fashion
        uart.BlockingTransmit(&send, 1);
        send++;

        // clear the display
        hw.display.Fill(false);

        // draw the title text
        char cstr[22];
        sprintf(cstr, "Uart DMA Test");
        hw.display.SetCursor(0, 0);
        hw.display.WriteString(cstr, Font_7x10, true);

        // draw the latest receive buffer contents
        sprintf(receive_str, "%d", pop);
        hw.display.SetCursor(0, 12);
        hw.display.WriteString(receive_str, Font_7x10, true);

        // update the display
        hw.display.Update();

        // wait 100 ms
        System::Delay(100);
    }
}
