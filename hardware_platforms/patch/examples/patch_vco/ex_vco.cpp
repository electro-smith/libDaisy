// Daisy Patch Example: VCO
// Author: Andrew Ikenberry
// Added: 12-2019

// Description: Eurorack VCO with variable waveform output(sine, tri, ramp, square). 
// 

// knob1 = coarse frequency
// knob2 = fine frequency
// knob3 = waveform
// knob4 = fm index
// cv2 = waveform
// cv3 = v/oct
// toggle = octave switch
// top row of LEDs = current waveform
// audio 1 out 1 = VCO output
// audio 1 out 2 = VCO output (dual mono)

// TODO:
// - make toggle hard/soft sync
// - implement sync
// - add buttons for octave + / -

#include "daisysp.h"
#include "daisy_patch.h"

using namespace daisy;
using namespace daisysp;

daisy_patch patch;
oscillator osc;

uint8_t waveforms[4] = {
    oscillator::WAVE_SIN, 
    oscillator::WAVE_TRI, 
    oscillator::WAVE_POLYBLEP_SAW, 
    oscillator::WAVE_POLYBLEP_SQUARE,
};

daisy_patch::led leds[4] = {
    patch.LED_A1,
    patch.LED_A2,
    patch.LED_A3,
    patch.LED_A4,
};

parameter coarse_knob, wave_knob, fine_knob, index_knob;
parameter voct_cv, wave_cv;

static void AudioCallback(float *in, float *out, size_t size)
{
    // running at control_rate (sample_rate / 48)
	float freq, sig, index;
	size_t wave;

    freq = coarse_knob.process() + fine_knob.process() + voct_cv.process();

    // convert midi note value to hz
    freq = mtof(freq);

    patch.toggle.Debounce();
    if (patch.toggle.Pressed())
    {
        freq *= 2;
    }

    wave = wave_knob.process() + wave_cv.process(); 
    if (wave > 3)
    {
        wave = 3;
    }
    osc.set_waveform(waveforms[wave]);

    patch.ClearLeds();
    patch.SetLed(leds[wave], 1);

    index = index_knob.process();

    // audio buffer running at sample_rate
    for (size_t i = 0; i < size; i += 2)
    {
        // read FM input, scale by index and add to freq
        freq += in[i] * index;
        osc.set_freq(freq);

    	sig = osc.process();

    	// left out
        out[i] = sig;
        // right out
        out[i+1] = sig;
    }
}

int main(void)
{
    patch.Init(); 
    osc.init(SAMPLE_RATE);
    osc.set_amp(.25);

    coarse_knob.init(patch.knob1, 10, 110, parameter::LINEAR); // coarse frequency
    wave_knob.init(patch.knob2, 0, 4, parameter::LINEAR); // waveform
    fine_knob.init(patch.knob3, -6, 6, parameter::LINEAR); // fine frequency
    index_knob.init(patch.knob4, 0, 100, parameter::LINEAR); // FM index
    voct_cv.init(patch.cv3, 0, 60, parameter::LINEAR); // volt per octave cv
    wave_cv.init(patch.cv2, 0, 4, parameter::LINEAR); // waveform cv

    dsy_adc_start();
    patch.StartAudio(AudioCallback);

    while(1) 
    {
        patch.UpdateLeds();
        dsy_system_delay(5);
    }
}
