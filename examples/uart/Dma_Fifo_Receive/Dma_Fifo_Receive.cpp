#include "daisy_patch.h"

using namespace daisy;

DaisyPatch hw;
UartHandler uart;

int main(void)
{
	// Initialize the Daisy Patch
	hw.Init();

	// Configure the Uart Peripheral
	UartHandler::Config uart_conf;
	uart_conf.periph = UartHandler::Config::Peripheral::USART_1;
	uart_conf.mode = UartHandler::Config::Mode::RX;
	uart_conf.pin_config.tx = Pin(PORTB, 6);
	uart_conf.pin_config.rx = Pin(PORTB, 7);

	// Initialize the Uart Peripheral
	uart.Init(uart_conf);

	// Start the FIFO Receive
	uart.DmaReceiveFifo();

	uint8_t pop = 0;
	while(1) {
		// if there's data, pop it from the FIFO
		if(uart.ReadableFifo()){
			pop = uart.PopFifo();
			hw.seed.SetLed(false);
		}	
		else{
			hw.seed.SetLed(true);
		}

		// clear the display
        hw.display.Fill(false);

		// draw the title text
        char cstr[26];
        sprintf(cstr, "Uart DMA Fifo Rx");
        hw.display.SetCursor(0, 0);
        hw.display.WriteString(cstr, Font_7x10, true);

		// draw the last popped data
        sprintf(cstr, "%d", pop);
        hw.display.SetCursor(0, 12);
        hw.display.WriteString(cstr, Font_7x10, true);
        
		// update the display
		hw.display.Update();

		// wait 100 ms
        System::Delay(100);
	}
}
