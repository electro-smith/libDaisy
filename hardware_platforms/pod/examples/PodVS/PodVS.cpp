#include "daisy_patch.h"
#include "daisysp.h"
#include <math.h>

using namespace daisy;
using namespace daisysp;

// midi note number to frequency helper
static float mtof(float m);

daisy_patch hw;
hid_ctrl   knob1;
oscillator osc;
static uint8_t wf;

static void audio(float *in, float *out, size_t size)
{
	float param, sig;
	// Check Switch to change waveform
	dsy_switch_debounce(&hw.button1);
	if(dsy_switch_falling_edge(&hw.button1))
	{
		wf = (wf + 1) % osc.WAVE_LAST; // increment and wrap
		osc.set_waveform(wf);
	}
	// Audio Loop
	for(size_t i = 0; i < size; i += 2)
	{
		param = knob1.process() * 72.0f;
		osc.set_freq(mtof(12.0f + param));
		sig	= osc.process();
		out[i] = out[i+1] = sig;
	}
}

int main(void)
{
	// Init Hardware (old style still)
	daisy_patch_init(&hw);

	// New C++ Inits:
	// Init Knob
	// If we build this into patch, it would be patch.knob1
	knob1.init(dsy_adc_get_rawptr(KNOB_1), SAMPLE_RATE);
	// Init Osc
	osc.init(SAMPLE_RATE);
	osc.set_waveform(osc.WAVE_SIN);

	// Old style still 
	dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audio);
	dsy_audio_start(DSY_AUDIO_INTERNAL);
	dsy_adc_start();
	while(1) {}
}

static float mtof(float m)
{
	return powf(2, (m - 69.0f) / 12.0f) * 440.0f;
}
