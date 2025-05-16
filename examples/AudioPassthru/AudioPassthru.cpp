/** Demonstration of running audio through the codec 
 *  Passes the audio input directly out to the audio output.
 */
#include "daisy_seed.h"

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

/** Global Hardware access */
DaisySeed hw;

/** Function that gets called at a regular interval by the hardware to 
 *  process, and/or generate audio signals
*/
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        /** Set each of our outputs to the value of this sine wave */
        OUT_L[i] = IN_L[i];
        OUT_R[i] = IN_R[i];
    }
}

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    /** Start the Audio engine, and call the "AudioCallback" function to fill new data */
    hw.StartAudio(AudioCallback);

    /** Infinite Loop */
    while(1) {}
}