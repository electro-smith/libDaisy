// # UART
// ## Description
// Uart Peripheral
//
// ## Credit
// Written by: shensley
// Date Added: March 2020
// ## TODO:
// - Add flexible config for:
//     - data size, stop bits, parity, baud, etc.
//     - dma vs interrupt (or not).
// - Error handling
// - Ring buffer queue for handling received data.
// - Transmit functions
// - Other UART Peripherals (currently only handles USART1 in UART mode.

#pragma once
#ifndef DSY_UART_H
#define DSY_UART_H
#include "daisy_core.h"

namespace daisy
{
class UartHandler
{
  public:
    UartHandler() {}
    ~UartHandler() {}

    // ## Functions
    // ### Init
    // Initializes the UART Peripheral
	// ~~~~
    void Init();
	// ~~~~

    // ### PollReceive
    // Reads the amount of bytes in blocking mode with a 10ms timeout.
    // ~~~~
    int PollReceive(uint8_t *buff, size_t size);
    // ~~~~
    // ### Receive
    // Reads the amount of bytes in non-blocking mode (using DMA).
    // TODO: Add some sort of flag for new data (plus resolve this as ring buffer comes in).
    // ~~~~
    int Receive(uint8_t *buff, size_t size);
    // ~~~~
	// ~~~~
    int CheckError();
	// ~~~~
//    bool Recieving();

  private:
};

} // namespace daisy

#endif
