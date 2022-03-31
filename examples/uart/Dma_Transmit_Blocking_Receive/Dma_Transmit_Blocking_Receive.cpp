#include "daisy_patch.h"

using namespace daisy;

DaisyPatch hw;
UartHandler uart;

// buffer to send from
uint8_t DMA_BUFFER_MEM_SECTION buff[256];

bool dma_ready = true;

// dma end callback, will start a new DMA transfer
void RestartUart(void* state, UartHandler::Result res)
{
    dma_ready = true;
}

int main(void)
{
    // start the Daisy Patch
    hw.Init();

    // init the tx buffer
    for(int i = 0; i < 256; i++)
    {
        buff[i] = i;
    }

    // set up our UART peripheral
    UartHandler::Config uart_conf;
    uart_conf.periph        = UartHandler::Config::Peripheral::USART_1;
    uart_conf.mode          = UartHandler::Config::Mode::TX_RX;
    uart_conf.pin_config.tx = Pin(PORTB, 6);
    uart_conf.pin_config.rx = Pin(PORTB, 7);

    // initialize the UART peripheral, and start reading
    uart.Init(uart_conf);

    uint8_t rx[4] = {0,0,0,0};
    while(1)
    {
        // blocking rx
        uart.BlockingReceive(rx, 4, 1000);

        if(dma_ready)
        {
            uart.DmaTransmit(buff, 256, NULL, RestartUart, NULL);
            dma_ready = false;
        }

        // clear the display
        hw.display.Fill(false);

        // draw the title text
        char cstr[22];
        sprintf(cstr, "Uart DMA Test");
        hw.display.SetCursor(0, 0);
        hw.display.WriteString(cstr, Font_7x10, true);

        // draw the receive buffer contents
        sprintf(cstr, "%d %d %d %d", rx[0], rx[1], rx[2], rx[3]);
        hw.display.SetCursor(0, 12);
        hw.display.WriteString(cstr, Font_7x10, true);

        // update the display
        hw.display.Update();

        // wait 100 ms
        System::Delay(100);
    }
}
