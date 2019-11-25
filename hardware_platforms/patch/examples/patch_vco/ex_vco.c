#include "daisysp.h"
#include "brd_patch.h"

static daisy_patch patch;
static dsy_oscillator osc;

static void audioCallback(float *in, float *out, size_t size)
{
	float sig, freq, amp;
	size_t wave;

	freq = dsy_adc_get_float(DSY_PATCH_KNOB_1);
	// scale signal to appropriate range / filter / etc.
	osc.freq = freq;

	wave = dsy_adc_get(DSY_PATCH_KNOB_2);
	// scale signal to appropriate range / filter / etc.
	osc.wave = wave;

	amp = dsy_adc_get_float(DSY_PATCH_KNOB_3);
	// scale signal to appropriate range / filter / etc.
	osc.amp = amp;

    for (size_t i = 0; i < size; i += 2)
    {
    	sig = dsy_oscillator_process(&osc);

    	// left out
        out[i] = sig;

        // right out
        out[i+1] = sig;
    }
}

int main(void)
{
	// initialize patch hardware and oscillator daisysp module
    daisy_seed_init(&patch.seed);
    daisy_patch_init(&patch);
    dsy_oscillator_init(&osc, DSY_AUDIO_SAMPLE_RATE);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
