/** Generation of a simple Audio signal */
#include "daisy_seed.h"
#include <cmath>

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

/** An increment for generating a 220Hz wave form at 48kHz sample rate
 *  Check out DaisySP for a more complete Oscillator implementation for your project.
 */
const float kSignalIncrement = (M_TWOPI * 220) * (1.0 / 48000);

/** Global Hardware access */
DaisySeed hw;

/** Global phase value for generated signal */
float phs;

/** Function that gets called at a regular interval by the hardware to 
 *  process, and/or generate audio signals
*/
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        /** 
         * Generate the next sample of our output signal 
         * 
         * This is a very basic sine-wave oscillator with a fixed increment to generate
         * a 220Hz waveform at 48kHz samplerate
         */
        float signal = sin(phs) * 0.5f;
        phs += kSignalIncrement;
        if(phs > M_TWOPI)
            phs -= M_TWOPI;

        /** Set each of our outputs to the value of this sine wave */
        OUT_L[i] = signal;
        OUT_R[i] = signal;
    }
}

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    /** Set our initial phs value for the oscillator to 0 */
    phs = 0.0;

    /** Start the Audio engine, and call the "AudioCallback" function to fill new data */
    hw.StartAudio(AudioCallback);

    /** Infinite Loop */
    while(1) {}
}