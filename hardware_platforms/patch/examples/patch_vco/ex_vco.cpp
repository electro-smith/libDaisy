// patch vco
// knob1 = coarse
// knob2 = fine
// knob3 = waveform
// knob4 = fm index
// toggle = naive or polyblep waveforms (TODO)
// bottom row of LEDs = current waveform (TODO)
// TODO (other) implement CVs
#include "daisysp.h"
#include "daisy_patch.h"

using namespace daisy;
using namespace daisysp;

daisy_patch patch;
oscillator osc;
parameter coarse_param, wave_param, fine_param, index_param;
Switch wave_toggle;

static void AudioCallback(float *in, float *out, size_t size)
{
	float freq, sig, index;
	size_t wave;
    bool wave_type;

    // read controls
    freq = mtof(coarse_param.process());
    freq  += mtof(fine_param.process());
    index = index_param.process();
    wave = wave_param.process();
    wave_type = wave_toggle.Pressed();
    wave_toggle.Debounce();

    // Set osc params
    osc.set_waveform(wave);

    // audio buffer
    for (size_t i = 0; i < size; i += 2)
    {
        // read FM input, scale by index and add to freq
        freq += in[i] * index;
        osc.set_freq(freq);

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
    //int num_waves = oscillator::WAVE_LAST - 1;

    // initialize objects
    patch.Init(); // initialize hardware (daisy seed, and patch)
    osc.init(SAMPLE_RATE); // init oscillator
    osc.set_amp(.25);
    wave_toggle.Init(TOGGLE_PIN, 100);

    // initialize controls
    coarse_param.init(patch.knob1, 10.0f, 110.0f, parameter::LINEAR); // coarse frequency
    wave_param.init(patch.knob2, 0.0, 4.0f, parameter::LINEAR); // waveform
    fine_param.init(patch.knob3, 0.0, 0.5f, parameter::LINEAR); // fine frequency
    index_param.init(patch.knob4, 0.0, 100.0f, parameter::LINEAR); // FM index

    // start adc and audio
    dsy_adc_start();
    patch.StartAudio(AudioCallback);

    while(1) {} // loop forever
}
