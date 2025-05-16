/** Example of using a quadrature encoder (like a PEC11)
 *  
 *  This will use D20 and D16 as the encoder's A and B inputs
 *  The click will be connected to D19
 *  The center pin on the encoder and the other side of the switch should be grounded.
 * 
 *  During this example, a value will be printed through the serial port
 *  The number will increase when rotating the encoder clockwise
 *  The number will decrease when rotating the encoder counter clockwise
 *  The number will reset to zero when the encoder is clicked.
 */
#include "daisy_seed.h"

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

/** Global Hardware access */
DaisySeed hw;

/** Global button object */
Encoder encoder;

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    /** Initialize our Encoder */
    encoder.Init(seed::D20, seed::D16, seed::D19);

    /** Start Serial COM Port logging via USB 
     *  The program will wait until a connection is made to move on from here.
     */
    hw.StartLog(true);
    hw.PrintLine("Turn or click the encoder to start");

    /** Create a variable to store the value we'll print out to USB */
    int output_value = 0;

    /** Infinite Loop */
    while(1)
    {
        /** The Encoder class uses "debouncing" to make sure that hardware activity 
         *  read into the Daisy matches what the user is doing, and prevents
         *  a lot of fast transitions that sometimes occur on hardware when engaging/disengaging 
         *  a switch, or rotating an encoder.
         */
        encoder.Debounce();

        /* This will return a -1 if the encoder was turned counter clockwise, or 
         * a 1 if the encoder was turned clockwise, or 0 if the encoder was not rotated.
         */
        int increment = encoder.Increment();

        if(increment > 0)
        {
            /** increase our output value and print it */
            output_value += 1;
            hw.PrintLine("Output Value:\t%d", output_value);
        }
        else if(increment < 0)
        {
            /** decrease our output value and print it */
            output_value -= 1;
            hw.PrintLine("Output Value:\t%d", output_value);
        }

        /** if the encoder was just clicked down, reset our output value to 0 */
        if(encoder.RisingEdge())
        {
            /** reset the output value to 0, and print it */
            output_value = 0;
            hw.PrintLine("Output Value:\t%d", output_value);
        }
    }
}