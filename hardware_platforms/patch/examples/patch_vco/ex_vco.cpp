#include "daisysp.h"
#include "daisy_patch.h"

using namespace daisy;
using namespace daisysp;

daisy_patch patch;
oscillator osc;
hid_ctrl knob1, knob2, knob3;

static void audioCallback(float *in, float *out, size_t size)
{
	float sig, freq, amp;
	size_t wave;
    for (size_t i = 0; i < size; i += 2)
    {
        // Read Knobs
        freq = 10.0f + (knob1.process() * 2000.0f); // scaled to 2kHz
        wave = knob2.process() * (osc.WAVE_LAST - 1);
        amp = knob3.process();
        // Set osc params
        osc.set_freq(freq);
        osc.set_waveform(wave);
        osc.set_amp(amp);
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
    knob1.init(patch.adc_ptr(KNOB_1), SAMPLE_RATE);
    knob2.init(patch.adc_ptr(KNOB_2), SAMPLE_RATE);
    knob3.init(patch.adc_ptr(KNOB_3), SAMPLE_RATE);
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback); // assign callback
    dsy_adc_start(); // start the ADCs to read values in the background.
    dsy_audio_start(DSY_AUDIO_INTERNAL); // start audio peripheral
    while(1) {} // loop forever
}
