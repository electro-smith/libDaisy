/** Example of using a CD4051 multiplexor to expand the ADC inputs */
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
     *  One channel configured for four inputs via CD4051 mux
     *  The Analog signal coming from the multiplexor is connected to pin A3
     *  The select pins from the Daisy are D12, and D31 (we only need two SEL pins since we're using 4 inputs)
     * 
     *  This example was made for the Daisy Seed2 DFM, but the pins can be swapped for other hardware.
     */
    AdcChannelConfig adc_cfg;
    adc_cfg.InitMux(seed::A3, 4, seed::D12, seed::D31);

    /** Initialize the ADC with our configuration */
    hw.adc.Init(&adc_cfg, 1);

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
         */
        System::Delay(250);
        hw.PrintLine("Input 1: %d", hw.adc.GetMux(0, 0));
        hw.PrintLine("Input 2: %d", hw.adc.GetMux(0, 1));
        hw.PrintLine("Input 3: %d", hw.adc.GetMux(0, 2));
        hw.PrintLine("Input 4: %d", hw.adc.GetMux(0, 3));
    }
}