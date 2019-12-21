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
    while(1)
    {
        dsy_system_delay(500);
        dsy_gpio_toggle(&hw.led);
        dsy_gpio_toggle(&hw.testpoint);
        sprintf(buff, "Tick:\t%d\n", hello_count);
        hw.usb_handle.TransmitInternal((uint8_t*)buff, strlen(buff));
    }
}
