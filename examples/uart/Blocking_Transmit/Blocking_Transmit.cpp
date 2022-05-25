#include "daisy_seed.h"

using namespace daisy;

DaisySeed    hw;
UartHandler uart;

int main(void)
{
    // Initialize the daisy hardware
    hw.Init();

    // Configure the Uart Peripheral
    UartHandler::Config uart_conf;
    uart_conf.periph        = UartHandler::Config::Peripheral::USART_1;
    uart_conf.mode          = UartHandler::Config::Mode::TX;
    uart_conf.pin_config.tx = Pin(PORTB, 6);
    uart_conf.pin_config.rx = Pin(PORTB, 7);

    // Initialize the uart peripheral and start the DMA transmit
    uart.Init(uart_conf);

    uint8_t tx = 0;
    while(1) {
        uart.BlockingTransmit(&tx, 1, 1000);
        tx++;
        System::Delay(200);
    }
}
