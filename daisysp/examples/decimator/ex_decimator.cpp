#include "daisysp.h"
#include "daisy_seed.h"

// Shortening long macro for sample rate
#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE
#endif

// Interleaved audio definitions
#define LEFT (i)
#define RIGHT (i+1)

using namespace daisysp;

static daisy_handle seed;
static oscillator osc;
static decimator decim;
static phasor phs;

static void audioCallback(float *in, float *out, size_t size)
{
	float osc_out, decimated_out;
    float downsample_amt;
    for (size_t i = 0; i < size; i += 2)
    {
        // Generate a pure sine wave
    	osc_out = osc.process();
        // Modulate downsample amount via phasor
        downsample_amt = phs.process();
        decim.set_downsample_factor(downsample_amt);
        // downsample and bitcrush
        decimated_out = decim.process(osc_out);
        // outputs
        out[LEFT] = decimated_out;
        out[RIGHT] = decimated_out;
    }
}

int main(void)
{
    // initialize seed hardware and daisysp modules
    daisy_seed_init(&seed);
    osc.init(SAMPLE_RATE);
    phs.init(SAMPLE_RATE, 0.5f);
    decim.init();

    // Set parameters for oscillator
    osc.set_waveform(osc.WAVE_SIN);
    osc.set_freq(220);
    osc.set_amp(0.25);
    // Set downsampling, and bit crushing values.
    decim.set_downsample_factor(0.4f);
    decim.set_bits_to_crush(8);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
