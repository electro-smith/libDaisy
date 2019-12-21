#include "daisysp.h"
#include "daisy_patch.h"

using namespace daisy;
using namespace daisysp;

daisy_patch patch;
oscillator osc;
parameter knob1, knob2, knob3;

static void audioCallback(float *in, float *out, size_t size)
{
	float sig, freq, fine;
	size_t wave;
    for (size_t i = 0; i < size; i += 2)
    {
        // Read Knobs
        freq = knob1.process(); 
        wave = knob2.process();
        fine = knob3.process();
        freq += fine;

        // Set osc params
        osc.set_freq(freq);
        osc.set_waveform(wave);

        // process
    	sig = osc.process();

    	// left out
        out[i] = sig;
        // right out
        out[i+1] = sig;
    }
}

int main(void)
{
    patch.init(); // initialize hardware (daisy seed, and patch)
    osc.init(SAMPLE_RATE); // init oscillator
    knob1.init(patch.ctrl(KNOB_1), 10, 2000, PARAM_CURVE_LOG);
    knob2.init(patch.ctrl(KNOB_2), 1, 4, PARAM_CURVE_LINEAR);
    knob3.init(patch.ctrl(KNOB_3), -10, 10, PARAM_CURVE_LINEAR);

    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback); // assign callback
    dsy_adc_start(); // start the ADCs to read values in the background.
    dsy_audio_start(DSY_AUDIO_INTERNAL); // start audio peripheral

    while(1) {} // loop forever
}
