// Daisy Patch Example: VCO
// Author: Andrew Ikenberry
// Added: 12-2019
// knob1 = coarse
// knob2 = fine
// knob3 = waveform
// knob4 = fm index
// cv2 = waveform
// cv3 = v/oct
// toggle = octave switch
// top row of LEDs = current waveform
// TODO: 
// - make v/oct cv accurate, and remove slew

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
    oscillator::WAVE_POLYBLEP_SQUARE 
};

// knob parameters
parameter coarse_knob, wave_knob, fine_knob, index_knob;

// CV parameters
parameter voct_cv, wave_cv;

static void AudioCallback(float *in, float *out, size_t size)
{
	float freq, fine, sig, index, voct;
	size_t wave;
    bool octave;

    // read controls
    freq = mtof(coarse_knob.process());
    fine = mtof(fine_knob.process());
    freq  += fine;
    index = index_knob.process();
    wave = wave_knob.process();
    patch.toggle.Debounce();
    octave = patch.toggle.Pressed();

    // read CV inputs
    voct = mtof(voct_cv.process());
    freq += voct;
    wave += wave_cv.process();
    if (wave > 4)
    {
        wave = 3;
    }

    // implement parameters
    if (octave)
    {
        freq *= 2;
    }

    // update waveform leds
    if (wave == 0)
    {
        patch.ClearLeds();
        patch.SetLed(patch.LED_A1, 1);
    } 
    else if (wave == 1)
    {
        patch.ClearLeds();
        patch.SetLed(patch.LED_A2, 1);
    } 
    else if (wave == 2)
    {
        patch.ClearLeds();
        patch.SetLed(patch.LED_A3, 1);
    } 
    else if (wave == 3)
    {
        patch.ClearLeds();
        patch.SetLed(patch.LED_A4, 1);
    }

    osc.set_waveform(waveforms[wave]);

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
    // initialize hardware and DaisySP modules
    patch.Init(); 
    patch.ClearLeds();
    osc.init(SAMPLE_RATE);
    osc.set_amp(.25);

    // initialize knob controls
    coarse_knob.init(patch.knob1, 10, 110, parameter::LINEAR); // coarse frequency
    wave_knob.init(patch.knob2, 0, 4, parameter::LINEAR); // waveform
    fine_knob.init(patch.knob3, -6, 6, parameter::LINEAR); // fine frequency
    index_knob.init(patch.knob4, 0, 100, parameter::LINEAR); // FM index

    // initialize CV inputs
    voct_cv.init(patch.cv3, 10, 110, parameter::EXP); // volt per octave
    wave_cv.init(patch.cv2, 0, 4, parameter::LINEAR); // waveform CV 

    // start adc and audio
    dsy_adc_start();
    patch.StartAudio(AudioCallback);

    while(1) 
    {
        patch.UpdateLeds();
        dsy_system_delay(5);
    }
}
