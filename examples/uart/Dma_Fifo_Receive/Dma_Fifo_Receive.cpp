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
    // Initialize the Daisy Patch
    hw.Init();

    // Configure the Uart Peripheral
    UartHandler::Config uart_conf;
    uart_conf.periph        = UartHandler::Config::Peripheral::USART_1;
    uart_conf.mode          = UartHandler::Config::Mode::RX;
    uart_conf.pin_config.tx = Pin(PORTB, 6);
    uart_conf.pin_config.rx = Pin(PORTB, 7);

    // Initialize the Uart Peripheral
    uart.Init(uart_conf);
    uart.DmaListenStart(uart_buffer, kUartBufferSize, uartCallback, nullptr);

    while(1)
    {
        // clear the display
        hw.display.Fill(false);

        // draw the title text
        char cstr[26];
        sprintf(cstr, "Uart DMA Fifo Rx");
        hw.display.SetCursor(0, 0);
        hw.display.WriteString(cstr, Font_7x10, true);

        // draw the last popped data
        hw.display.SetCursor(0, 12);
        hw.display.WriteString(receive_str, Font_7x10, true);

        // update the display
        hw.display.Update();

        // wait 100 ms
        System::Delay(100);
    }
}
