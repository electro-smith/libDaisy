/** Daisy Blink Example for external DFU bootloader
 *
 *  This example blinks the Daisy Seed's LED
 *
 *  This project is made to demonstrate testing DAisy's v6 bootloader
 *  with external USB DFU configuration.
 *
 *  The application can use any method it wants to jump to the bootloader.
 *  With the infinite timeout below, the unit will stay in the bootloader
 *  indefinitely, or until power cycled.
 *
 *  As a test, you can recompile this project with different blink delays
 *
 *  To enter the bootloader connect D27 to GND, and then release.
 *  This is SW 1 on the Daisy Pod, but can be replaced with any method of your preference.
 *
 */
#include "daisy_seed.h"

using namespace daisy;

DaisySeed hardware;
Switch bootloader_switch;

int main()
{
    hardware.Init();
    bool blinkState = false;

    /** Initialize the boot switch */
    bootloader_switch.Init(seed::D27);

    uint32_t blink_rate = 50; /**< Change me to test tweaking the program! */

    /** Timing variables */
    uint32_t now, blink_time;
    now = blink_time = System::GetNow();

    while (true)
    {
        now = System::GetNow();

        /** Blink without blocking the loop */
        if (now - blink_time > blink_rate)
        {
            hardware.SetLed(blinkState);
            blinkState = !blinkState;
            blink_time = now;
        }

        /** debounce the button and check for release */
        bootloader_switch.Debounce();
        if (bootloader_switch.FallingEdge())
        {
            System::ResetToBootloader(System::DAISY_INFINITE_TIMEOUT);
        }

        /** 1ms delay for predictable debouncing time */
        System::Delay(1);
    }
}
