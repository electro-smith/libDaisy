#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;

static daisy_handle seed;
static tone flt;
static oscillator osc, lfo;

static void audioCallback(float *in, float *out, size_t size)
{
	float saw, freq, output;
    for (size_t i = 0; i < size; i += 2)
    {
        freq = lfo.process()*5000;
    	saw = osc.process();

        flt.set_freq(freq);
        output = flt.process(saw);

    	// left out
        out[i] = output;

        // right out
        out[i+1] = output;
    }
}

int main(void)
{
	// initialize seed hardware and daisysp modules
    daisy_seed_init(&seed);

    // initialize tone object
    flt.init(DSY_AUDIO_SAMPLE_RATE);

    // set parameters for sine oscillator object
    lfo.init(DSY_AUDIO_SAMPLE_RATE);
    lfo.set_waveform(oscillator::WAVE_TRI);
    lfo.set_amp(0.25);
    lfo.set_freq(.2);

    // set parameters for sine oscillator object
    osc.init(DSY_AUDIO_SAMPLE_RATE);
    osc.set_waveform(oscillator::WAVE_POLYBLEP_SAW);
    osc.set_freq(100);
    osc.set_amp(0.25);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
