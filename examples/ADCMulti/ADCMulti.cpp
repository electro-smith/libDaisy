/** Example of initializing multiple ADC inputs */
#include "daisy_seed.h"

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

/** Global Hardware access */
DaisySeed hw;

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    /** Configure the ADC
     * 
     *  Three CV inputs (-5V to 5V -> 3V3 to 0V)
     *  A0, A6, and A2
     * 
     *  This example was made for the Daisy Seed2 DFM, but the pins can be swapped for other hardware.
     */
    AdcChannelConfig adc_cfg[3];
    adc_cfg[0].InitSingle(seed::A0);
    adc_cfg[1].InitSingle(seed::A6);
    adc_cfg[2].InitSingle(seed::A2);

    /** Initialize the ADC with our configuration */
    hw.adc.Init(adc_cfg, 3);

    /** Start the ADC conversions in the background */
    hw.adc.Start();

    /** Startup the USB Serial port */
    hw.StartLog();

    /** Infinite Loop */
    while(1)
    {
        /** Print the values via Serial every 250ms 
         *  Values will be 0 when inputs are 0V
         *  Values will be 65536 when inputs are 3v3
         * 
         *  Based on the CV input circuit this means that
         *  Values will be 0 when input is 5V
         *  Values will be ~32768 when input is ~0V
         *  Values will be 65536 when input is -5V
         */
        System::Delay(250);
        hw.PrintLine("Input 1: %d", hw.adc.Get(0));
        hw.PrintLine("Input 2: %d", hw.adc.Get(1));
        hw.PrintLine("Input 3: %d", hw.adc.Get(2));
    }
}