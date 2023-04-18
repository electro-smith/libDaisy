/** Example of DAC Output using basic Polling implementation
 *  
 *  This will use A8 to demonstrate DAC output
 *  This will generate a ramp wave from 0-3v3
 * 
 *  A7 will output a ramp, and output A8 will output an inverted ramp
 */
#include "daisy_seed.h"

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

/** Global Hardware access */
DaisySeed hw;

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    /** Configure and Initialize the DAC */
    DacHandle::Config dac_cfg;
    dac_cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
    dac_cfg.buff_state = DacHandle::BufferState::ENABLED;
    dac_cfg.mode       = DacHandle::Mode::POLLING;
    dac_cfg.chn        = DacHandle::Channel::BOTH;
    hw.dac.Init(dac_cfg);

    /** Variable for output */
    int output_val_1 = 0;
    int output_val_2 = 0;

    /** Infinite Loop */
    while(1)
    {
        /** Every 1millisecond we'll increment our 12-bit value
         *  The value will go from 0-4095
         *  The full ramp waveform will have a period of about 4 seconds
         *  The secondary waveform will be an inverted ramp (saw) 
         *  at the same frequency
         */
        System::Delay(1);

        /** increment our output value, and wrap around if it goes out range. */
        output_val_1 += 1;
        if(output_val_1 > 4095)
            output_val_1 = 0;

        /** This sets our second output to the opposite of the first */
        output_val_2 = 4095 - output_val_1;

        /** And write our 12-bit value to the output */
        hw.dac.WriteValue(DacHandle::Channel::ONE, output_val_1);
        hw.dac.WriteValue(DacHandle::Channel::TWO, output_val_2);
    }
}