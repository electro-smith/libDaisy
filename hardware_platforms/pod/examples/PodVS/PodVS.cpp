#include "daisy_patch.h"
#include "daisysp.h"
#include <math.h>

daisy_patch	hw;
dsy_ctrl	   k, cv;
dsy_oscillator osc;
uint8_t		   wf;

static float mtof(float m);

static void audio(float *in, float *out, size_t size)
{
	float param, sig;
	dsy_switch_debounce(&hw.button1);
	if(dsy_switch_falling_edge(&hw.button1))
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
		param	  = dsy_ctrl_process(&k) * 72.0f; // 6 octave knob
		param += dsy_ctrl_process(&cv) * 60.0f; // 5 octave CV (1V/Octave)
		param = DSY_CLAMP(param, 0.0f, 127.0f); 
		osc.freq = mtof(12.0f + param);

		osc.amp	= 0.4f;
		sig		   = dsy_oscillator_process(&osc);
		out[i]	 = sig;
		out[i + 1] = sig;
	}
}

int main(void)
{
	daisy_patch_init(&hw);
	wf	 = 0;
	dsy_ctrl_init_knob(&k, dsy_adc_get_rawptr(KNOB_1), SAMPLE_RATE);
	dsy_ctrl_init_bipolar_cv(&cv, dsy_adc_get_rawptr(CV_2), SAMPLE_RATE);
	dsy_oscillator_init(&osc, SAMPLE_RATE);
	dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audio);
	dsy_audio_start(DSY_AUDIO_INTERNAL);
	dsy_adc_start();
	while(1) {}
}

static float mtof(float m) 
{
	return powf(2, (m - 69.0f) / 12.0f) * 440.0f;
}
