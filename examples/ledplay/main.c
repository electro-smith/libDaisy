// Short Example showing how to write to the LEDs on Daisy Patch
#include "libdaisy.h"
#include "daisysp.h"
#include "dsy_patch_bsp.h"

static void clear_leds();

int main(void)
{
    uint8_t selected_led = 0;
    uint8_t board = DSY_SYS_BOARD_DAISY_SEED;
    dsy_system_init(board);
    dsy_led_driver_init(board);
    while(1) 
    {
        dsy_system_delay(100);
        selected_led += 1;
        if (selected_led > LED_LAST - 1)
        {
            selected_led = 0;
        }
        clear_leds();
        dsy_led_driver_set_led(selected_led, 1.0f);
        dsy_led_driver_update();
    }
}

// Sets all LEDs to be off.
static void clear_leds()
{
    for (int i = 0; i < LED_LAST; i++)
    {
        dsy_led_driver_set_led(i, 0.0f);    
    }
}
