/** Example of setting up a switch
 *  
 *  This will use D9 on one side of the switch
 *  The other side of the switch should be grounded.
 * 
 *  Pressing, or engaging the switch will turn on the built-in LED
 */
#include "daisy_seed.h"

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

/** Global Hardware access */
DaisySeed hw;

/** Global button object */
Switch button;

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    /** Initialize our Toggle */
    button.Init(seed::D9);

    /** Infinite Loop */
    while(1)
    {
        /** The Switch class uses "debouncing" to make sure that button activity 
         *  read into the Daisy matches what the user is doing, and prevents
         *  a lot of fast transitions that sometimes occur on hardware when engaging/disengaging 
         *  a switch.
         */
        button.Debounce();

        /** Now we check if it's pressed, and turn the LED on or not
         * 
         *  This could be simplified to: 
         *  hw.SetLed(button.Pressed());
         */
        if(button.Pressed())
            hw.SetLed(true);
        else
            hw.SetLed(false);
    }
}