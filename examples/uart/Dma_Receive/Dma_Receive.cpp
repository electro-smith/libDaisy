#include "daisy_patch.h"

using namespace daisy;

DaisyPatch  hw;
UartHandler uart;

// buffer to read into
uint8_t DMA_BUFFER_MEM_SECTION buff[4];

void RestartUart(void* state, UartHandler::Result res);

// dma end callback, will start a new DMA transfer
void RestartUart(void* state, UartHandler::Result res)
{
    uart.DmaReceive(buff, 4, NULL, RestartUart, NULL);
}

int main(void)
{
    // start the Daisy Patch
    hw.Init();

    // reset our receive buffer
    for(int i = 0; i < 4; i++)
    {
        buff[i] = 0;
    }

    // set up our UART peripheral
    UartHandler::Config uart_conf;
    uart_conf.periph        = UartHandler::Config::Peripheral::USART_1;
    uart_conf.mode          = UartHandler::Config::Mode::RX;
    uart_conf.pin_config.tx = Pin(PORTB, 6);
    uart_conf.pin_config.rx = Pin(PORTB, 7);

    // initialize the UART peripheral, and start reading
    uart.Init(uart_conf);
    uart.DmaReceive(buff, 4, NULL, RestartUart, NULL);

    while(1)
    {
        // clear the display
        hw.display.Fill(false);

        // draw the title text
        char cstr[22];
        sprintf(cstr, "Uart DMA Rx Test");
        hw.display.SetCursor(0, 0);
        hw.display.WriteString(cstr, Font_7x10, true);

        // draw the receive buffer contents
        sprintf(cstr, "%d %d %d %d", buff[0], buff[1], buff[2], buff[3]);
        hw.display.SetCursor(0, 12);
        hw.display.WriteString(cstr, Font_7x10, true);

        // update the display
        hw.display.Update();

        // wait 100 ms
        System::Delay(100);
    }
}
