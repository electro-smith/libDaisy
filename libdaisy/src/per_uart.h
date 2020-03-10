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

    void Init();

    int PollReceive(uint8_t *buff, size_t size);
    int Receive(uint8_t *buff, size_t size);
    int CheckError();
//    bool Recieving();

  private:
};

} // namespace daisy

#endif
