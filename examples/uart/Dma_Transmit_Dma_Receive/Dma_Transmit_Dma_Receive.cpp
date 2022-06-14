#include "daisy_field.h"
#include "daisy_patch.h"

using namespace daisy;

// DaisyField  hw;
DaisyPatch  hw;
UartHandler uart;

// rx / tx buffers
uint8_t DMA_BUFFER_MEM_SECTION rx_buff[4];
uint8_t DMA_BUFFER_MEM_SECTION tx_buff[10];

void RestartUartTx(void* state, UartHandler::Result res);

void RestartUartRx(void* state, UartHandler::Result res){
}

void RestartUartTx(void* state, UartHandler::Result res)
{
    uart.DmaReceive(rx_buff, 4, NULL, RestartUartRx, NULL);
    uart.DmaTransmit(tx_buff, 10, NULL, RestartUartTx, NULL);
}

int main(void)
{
    // start the Daisy Patch
    hw.Init();

    // reset our dma buffers
    for(int i = 0; i < 10; i++)
    {
        tx_buff[i] = i;
    }

    // set up our UART peripheral
    UartHandler::Config uart_conf;
    uart_conf.periph        = UartHandler::Config::Peripheral::USART_1;
    uart_conf.mode          = UartHandler::Config::Mode::TX_RX;
    uart_conf.pin_config.tx = Pin(PORTB, 6);
    uart_conf.pin_config.rx = Pin(PORTB, 7);

    // initialize the UART peripheral, and start reading
    uart.Init(uart_conf);

    uart.DmaReceive(rx_buff, 4, NULL, NULL, NULL);
    uart.DmaTransmit(tx_buff, 10, NULL, RestartUartTx, NULL);
    while(1)
    {
        // clear the display
        hw.display.Fill(false);

        // draw the title text
        char cstr[22];
        sprintf(cstr, "Uart DMA TxRx Test");
        hw.display.SetCursor(0, 0);
        hw.display.WriteString(cstr, Font_7x10, true);

        // draw the receive buffer contents
        sprintf(cstr, "%d %d %d %d", rx_buff[0], rx_buff[1], rx_buff[2], rx_buff[3]);
        hw.display.SetCursor(0, 12);
        hw.display.WriteString(cstr, Font_7x10, true);

        // update the display
        hw.display.Update();

        // wait 100 ms
        System::Delay(100);
    }
}
