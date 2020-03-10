#include "daisy_patch.h"

using namespace daisy;

daisy_patch hw;
UartHandler uart;
uint8_t     mybuff[16];

int main(void)
{
    hw.Init();
    uart.Init();
    uart.Receive(mybuff, 3);
    for(;;) {}
}
