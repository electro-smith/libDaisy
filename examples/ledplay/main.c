// Short Example showing how to write to the LEDs on Daisy Patch
#include "dsy_seed.h"
#include "dsy_patch_bsp.h"

daisy_patch patch;

static void clear_leds();

int main(void)
{
    uint8_t selected_led = 0;
    uint8_t addr = 0x00; // led driver address
    daisy_seed_init(&patch.seed);
    daisy_patch_init(&patch);
    dsy_led_driver_init(&patch.seed.i2c1_handle, &addr, 1);
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
