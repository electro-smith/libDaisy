#include "daisy_seed.h"

using namespace daisy;

DaisySeed    hw;
UartHandler uart;

#define BUFF_SIZE 256

// buffer to be dma transmitted
static uint8_t DMA_BUFFER_MEM_SECTION buff[BUFF_SIZE];

void RestartUart(void* state, UartHandler::Result res);

// dma end callback, will start a new DMA transfer
void RestartUart(void* state, UartHandler::Result res)
{
    if(res != UartHandler::Result::ERR)
    {
        uart.DmaTransmit(buff, BUFF_SIZE, NULL, RestartUart, NULL);
    }
}

int main(void)
{
    // Initialize the daisy hardware
    hw.Init();

    // initilize the buffer
    for(int i = 0; i < BUFF_SIZE; i++)
    {
        buff[i] = i;
    }

    // Configure the Uart Peripheral
    UartHandler::Config uart_conf;
    uart_conf.periph        = UartHandler::Config::Peripheral::USART_1;
    uart_conf.mode          = UartHandler::Config::Mode::TX;
    uart_conf.pin_config.tx = Pin(PORTB, 6);
    uart_conf.pin_config.rx = Pin(PORTB, 7);

    // Initialize the uart peripheral and start the DMA transmit
    uart.Init(uart_conf);
    uart.DmaTransmit(buff, BUFF_SIZE, NULL, RestartUart, NULL);

    // loop forever
    while(1) {}
}
