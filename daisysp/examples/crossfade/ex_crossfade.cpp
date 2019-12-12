#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;

static daisy_handle seed;
static crossfade cfade;
static oscillator osc_sine, osc_saw, lfo;

static void audioCallback(float *in, float *out, size_t size)
{
	float saw, sine, pos, output;
    for (size_t i = 0; i < size; i += 2)
    {
    	sine = osc_sine.process();
        saw = osc_saw.process();

        // lfo output = -1 to 1
        pos = lfo.process();

        // scale signal between 0 and 1
        pos = (pos + 1) / 2;

        cfade.set_pos(pos);
        output = cfade.process(sine, saw);

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

    // set params for crossfade object
    cfade.init();
    cfade.set_curve(CROSSFADE_LIN);

    // set parameters for sine oscillator object
    osc_sine.init(DSY_AUDIO_SAMPLE_RATE);
    osc_sine.set_waveform(oscillator::WAVE_SIN);
    osc_sine.set_freq(100);
    osc_sine.set_amp(0.25);

    // set parameters for sawtooth oscillator object
    osc_saw.init(DSY_AUDIO_SAMPLE_RATE);
    osc_saw.set_waveform(oscillator::WAVE_POLYBLEP_SAW);
    osc_saw.set_freq(100);
    osc_saw.set_amp(0.25);

    // set parameters for triangle lfo oscillator object
    lfo.init(DSY_AUDIO_SAMPLE_RATE);
    lfo.set_waveform(oscillator::WAVE_TRI);
    lfo.set_freq(.25);
    lfo.set_amp(1);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
