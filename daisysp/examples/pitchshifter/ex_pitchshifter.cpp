#include "daisysp.h"
#include "daisy_seed.h"

// Example that takes the mono input from channel 1 (left input), 
// and pitchshifts it up 1 octave.
// The left output will be pitchshifteed, while the right output
// stays will be the unshifted left input.

// Defines for Interleaved Audio
#define LEFT (i)
#define RIGHT (i+1)

using namespace daisysp;

daisy_handle seed;
pitchshifter ps;

static void audioCallback(float *in, float *out, size_t size)
{
	float shifted, unshifted;
    for (size_t i = 0; i < size; i += 2)
    {
        unshifted = in[LEFT];
        shifted = ps.process(unshifted);
        out[LEFT] = shifted;
        out[RIGHT] = unshifted;
    }
}

int main(void)
{
	// initialize seed hardware and daisysp modules
    daisy_seed_init(&seed);

    ps.init(DSY_AUDIO_SAMPLE_RATE);
    // set transposition 1 octave up (12 semitones)
    ps.set_transposition(12.0f);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
