/** Example of setting up a Gate Input
 * 
 *  This connects a BJT Gate Input circuit to the D11 pin
 *  The LED will indicate the State of the input.
 */
#include "daisy_seed.h"

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

/** Global Hardware access */
DaisySeed hw;

/** Global gate input object */
GateIn gate_in;

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    /** Initialize our Gate Input */
    gate_in.Init(seed::D11);


    /** Infinite Loop */
    while(1)
    {
        /** If the Gate Input signal is high, we light up the LED 
         *  This could also be written as: hw.SetLed(gate_in.State())
         */
        if(gate_in.State())
            hw.SetLed(true);
        else
            hw.SetLed(false);
    }
}