// GPIO Input
// Example of using a simple button
//
// Setup:
// * Connect one end of a button to GND
// * Connect other end to pin D0 on the Daisy Seed
//
#include "daisy_seed.h"

using namespace daisy;
using namespace daisy::seed;

DaisySeed hw;

int main(void)
{
    // Initialize the Daisy Seed
    hw.Init();

    // Start the Serial Logger
    hw.StartLog();

    // Create a GPIO object
    GPIO my_button;

    // Initialize the GPIO object
    // Mode: INPUT - because we want to read from the button
    // Pullup: Internal resistor to prevent needing extra components
    my_button.Init(D0, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

    while(1)
    {
        // And let's store the state of the button in a variable called "button_state"
        bool button_state = my_button.Read();

        // Set the USER LED to the button state
        hw.SetLed(button_state);

        // Print the Button state
        hw.PrintLine("Button State: %s", button_state ? "true" : "false");
    }
}
