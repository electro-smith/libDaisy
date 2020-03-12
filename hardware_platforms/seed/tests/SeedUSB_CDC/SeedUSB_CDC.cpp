#include <stdio.h>
#include <string.h>
#include "daisy_seed.h"

using namespace daisy;

static daisy_handle hw;

UsbHandle usb_handle;
int main(void)
{
    // Initialize Hardware
    daisy_seed_init(&hw);
    //	dsy_tim_start();
    int   hello_count = 0;
    char  buff[512];

    uint8_t *mbuff;
    mbuff = (uint8_t*)malloc(8);

    while(1)
    {
        dsy_system_delay(500);
        dsy_gpio_toggle(&hw.led);
        dsy_gpio_toggle(&hw.testpoint);
        sprintf(buff, "Tick:\t%d\r\n", hello_count);
        hello_count = (hello_count + 1) % 100;
        hw.usb_handle.TransmitInternal((uint8_t*)buff, strlen(buff));
    }
}
