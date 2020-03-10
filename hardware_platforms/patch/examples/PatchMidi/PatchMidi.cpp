#include "daisy_patch.h"
//#include "daisy.h"
//#include "sys_dma.h"
//#include "daisysp.h"

using namespace daisy;
//using namespace daisysp;

#define LEFT (i)
#define RIGHT (i + 1)

#define NUM_VOICES 32
#define MAX_DELAY ((size_t)(10.0f * SAMPLE_RATE))

// Hardware
daisy_patch hw;
UartHandler uart;
uint8_t     mybuff[16];
int         err;

static uint8_t reverse(uint8_t b)
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}


int main(void)
{
    // Init everything.
    hw.Init();
    uart.Init();
    err = uart.Receive(mybuff, 3);

    for(;;)
    {
        err = uart.CheckError();
    }
}
