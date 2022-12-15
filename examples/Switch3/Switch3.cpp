/** Example of setting up a three way (ON-OFF-ON) switch and reading it's position 
 *  
 *  This will use D29, and D30 on the two outside legs of the switch.
 *  The center leg should be grounded.
 * 
 *  When the toggle is in the left position, the onboard USER LED will be off
 *  When the toggle is in the center position, the onboard USER LED will blink slowly
 *  When the toggle is in the right position, the onboard USER LED will blink quickly
 */
#include "daisy_seed.h"

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

/** Global Hardware access */
DaisySeed hw;

/** Global Toggle object */
Switch3 toggle;

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    /** Initialize our Toggle */
    toggle.Init(seed::D29, seed::D30);

    /** Infinite Loop */
    while(1)
    {
        /** TODO: simplify and/or explain.
         *  These variable use the system millisecond timer to toggle between true/false
         *  The (now & number) will cause the millisecond counter to be looped in the scope of 0-that value 
         *  These have to be values that function as bit-masks (i.e. (2^x)-1) for this to work
         * 
         *  Proposed simplification 1: just use modulo?
         *  Proposed simplification 2: use delays for each blink below? (would require making the example quite a bit longer)
         */
        bool slow_blink = (System::GetNow() & 1023) < 511;
        bool fast_blink = (System::GetNow() & 511) < 255;

        /** Read the toggle position, and to the expected blink action */
        switch(toggle.Read())
        {
            case Switch3::POS_LEFT: hw.SetLed(false); break;
            case Switch3::POS_CENTER: hw.SetLed(slow_blink); break;
            case Switch3::POS_RIGHT: hw.SetLed(fast_blink); break;
        }
    }
}