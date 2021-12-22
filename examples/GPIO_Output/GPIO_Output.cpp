// GPIO Output
// Example of toggling an LED on/off
//
// Setup:
// * Connect the CATHODE (negative leg) of the LED to GND
// * Connect the ANODE (positive leg, usually longer) of the LED to one side of a resistor (1K)
// * Connect other end of resistor to pin D1 on the daisy.
//
#include "daisy_seed.h"

using namespace daisy;
using namespace daisy::seed;

DaisySeed hw;

int main(void)
{
    // Initialize the Daisy Seed hardware
    hw.Init();

    // Create an LED
    GPIO my_led;

    // Initialize it to pin D1 as an OUTPUT
    my_led.Init(D1, GPIO::Mode::OUTPUT);

    // In an infinite loop, we'll continuously turn the LED on/off.
    while(1)
    {
        // Set the pin HIGH
        my_led.Write(true);
        // Wait half a second (500ms)
        System::Delay(500);
        // Set the pin LOW
        my_led.Write(false);
        // Wait another half a second (500ms)
        System::Delay(500);
        // You can also use Toggle to change the state
        my_led.Toggle();
        // Wait another half a second (500ms)
        System::Delay(500);
        // And once more to flip it back
        my_led.Toggle();
    }
}
