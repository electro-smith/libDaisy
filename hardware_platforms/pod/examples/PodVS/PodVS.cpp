#include "daisy_pod.h"
#include "daisysp.h"
#include <math.h>

const float freqmin = logf(20.0f);
const float freqmax = logf(20000.0f);

daisy_pod hw;
dsy_knob k;
dsy_switch *   button;
dsy_oscillator osc;
uint8_t wf;

static void audio(float *in, float *out, size_t size) 
{
	float param, sig;
	dsy_switch_debounce(button);
	if(dsy_switch_falling_edge(button)) 
	{
		wf += 1;
		if(wf > DSY_OSC_WAVE_LAST - 1) 
		{
			wf = 0;
		}
		osc.wave = (dsy_oscillator_waveform)wf;
	}
	for(size_t i = 0; i < size; i += 2) 
	{
		param = dsy_knob_process(&k);
		osc.freq   = expf((param * (freqmax - freqmin)) + freqmin);
		sig = dsy_oscillator_process(&osc);
		out[i]	 = sig;
		out[i + 1] = sig;
	}
}

int main(void)
{
	daisy_pod_init(&hw);
	button = &hw.switches[SW_1];
	wf	 = 0;
	dsy_knob_init(&k,
				  dsy_adc_get_rawptr(KNOB_1),
				  0.01f,
				  DSY_KNOB_THRESH_LOCK,
				  SAMPLE_RATE);
	dsy_oscillator_init(&osc, SAMPLE_RATE);
	dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audio);
	dsy_audio_start(DSY_AUDIO_INTERNAL);
	dsy_adc_start();
	while(1)
	{
	}
}
