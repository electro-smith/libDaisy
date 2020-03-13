#include <stdio.h>
#include <string.h>
#include "daisy_seed.h"
#include "hid_oled_display.h"

using namespace daisy;

static daisy_handle hw;
OledDisplay         display;

int main(void)
{
    daisy_seed_init(&hw);
    display.Init();
    bool state;
    state = false;
    while(1) {
        dsy_system_delay(250);
        display.Fill(state);
        state = !state;
        display.Update();
    }
}

