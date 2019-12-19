#include "daisysp.h"
#include "daisy_seed.h"
#include <algorithm>

// Shortening long macro for sample rate
#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE
#endif

// Interleaved audio definitions
#define LEFT (i)
#define RIGHT (i+1)

using namespace daisysp;

static daisy_handle seed;

// Helper Modules
static metro tick;
static pluck plk;

// MIDI note numbers for a major triad
const float arpeggio[3] = { 36.0f, 40.0f, 45.0f };
uint8_t arp_idx;

static void audioCallback(float *in, float *out, size_t size)
{
	float sig_out, freq, trig;
    for (size_t i = 0; i < size; i += 2)
    {
        // When the metro ticks: 
        // advance the arpeggio, and trigger the pluck.
        trig = 0.0f;
        if (tick.process())
        {
            freq = mtof(arpeggio[arp_idx]); // convert midi nn to frequency.
            arp_idx = arp_idx + 1 % 3; // advance the arpeggio, wrapping at the end.
            plk.set_freq(freq);
            trig = 1.0f;
        }
        sig_out = plk.process(trig);
        // Output
        out[LEFT] = sig_out;
        out[RIGHT] = sig_out;
    }
}

int main(void)
{
    float init_buff[256]; // buffer for pluck impulse

    // initialize seed hardware and daisysp modules
    daisy_seed_init(&seed);

    // Set up metro to pulse every second
    tick.init(1.0f, SAMPLE_RATE);    
    // Set up pluck algo
    plk.init(SAMPLE_RATE, init_buff, 256, PLUCK_MODE_RECURSIVE);
    plk.set_decay(0.65f);
    plk.set_amp(0.3f);

    arp_idx = 0;

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
