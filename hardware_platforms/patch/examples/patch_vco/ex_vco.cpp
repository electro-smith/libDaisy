#include "daisysp.h"
#include "daisy_patch.h"

using namespace daisy;
using namespace daisysp;

daisy_patch patch;
oscillator osc;
parameter freqctrl, wavectrl, ampctrl;

static void AudioCallback(float *in, float *out, size_t size)
{
	float sig, freq, amp;
	size_t wave;
    for (size_t i = 0; i < size; i += 2)
    {
        // Read Knobs
        freq = mtof(freqctrl.process());
        wave = wavectrl.process();
        amp = ampctrl.process();
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
    int num_waves = oscillator::WAVE_LAST - 1;
    patch.Init(); // initialize hardware (daisy seed, and patch)
    osc.init(SAMPLE_RATE); // init oscillator

    // This is with the GetCtrl, but it can also be done with the public members.
    //freqctrl.init(patch.GetCtrl(daisy_patch::KNOB_1), 10.0, 110.0, parameter::LINEAR);
    //wavectrl.init(patch.GetCtrl(daisy_patch::KNOB_2), 0.0, num_waves, parameter::LINEAR);
    //ampctrl.init(patch.GetCtrl(daisy_patch::KNOB_2), 0.0, 0.5, parameter::LINEAR);
    // Like this:
    freqctrl.init(patch.knob1, 10.0, 110.0f, parameter::LINEAR);
    wavectrl.init(patch.knob2, 0.0, num_waves, parameter::LINEAR);
    ampctrl.init(patch.knob3, 0.0, 0.5f, parameter::LINEAR);

    dsy_adc_start();
    patch.StartAudio(AudioCallback);

    while(1) {} // loop forever
}
