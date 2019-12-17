#include "daisy_patch.h"
#include "daisysp.h"
#include <math.h>

#define MYPREFERENCE

using namespace daisy;
using namespace daisysp;

//static float mtof(float m);

daisy_patch	hw;
oscillator	 osc;
whitenoise	 nse;
static uint8_t wf;
parameter	  param_freq, param_nse_amp, param_osc_amp, param_bright;
parameter	  param_ampcv;

static float freq;
static void audio(float *in, float *out, size_t size)
{
	float sig, namp, oamp;
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
		// Get Parameters
		param_bright.process(); // for LEDs below
		freq  = param_freq.process();
		namp  = param_nse_amp.process();
		oamp  = param_osc_amp.process();
		oamp += param_ampcv.process();
		namp += (param_ampcv.value() * param_ampcv.value()); // exp only for noise
		// Set module parameters
		osc.set_freq(freq);
		osc.set_amp(oamp);
		nse.set_amp(namp);
		// Process
		sig	= osc.process();
		sig += nse.process();
		out[i] = out[i+1] = sig;
	}
}

int main(void)
{
	// Initialize Hardware
	hw.init();
#ifdef MYPREFERENCE
	param_freq.init(hw.ctrl(KNOB_1), 10.0f, 20000.0f, PARAM_CURVE_LOG);
	param_nse_amp.init(hw.ctrl(KNOB_2), 0.0f, 1.0f, PARAM_CURVE_EXP);
	param_osc_amp.init(hw.ctrl(KNOB_3), 0.0f, 0.4f, PARAM_CURVE_LINEAR);
	param_bright.init(hw.ctrl(KNOB_4), 0.0f, 1.0f, PARAM_CURVE_CUBE);
	param_ampcv.init(hw.ctrl(CV_2), 0.0f, 1.0f, PARAM_CURVE_LINEAR);
#else
	param_freq.init(hw.knob1, 20.0f, 20000.0f, PARAM_CURVE_LOG);
	param_nse_amp.init(hw.knob2, 0.0f, 1.0f, PARAM_CURVE_EXP);
	param_osc_amp.init(hw.knob3, 0.0f, 0.4f, PARAM_CURVE_LINEAR);
	param_bright.init(hw.knob4, 0.0f, 1.0f, PARAM_CURVE_CUBE);
#endif
	// Init Osc and Nse
	dsy_tim_start();
	osc.init(SAMPLE_RATE);
	nse.init();
	// Old style still 
	dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audio);
	dsy_audio_start(DSY_AUDIO_INTERNAL);
	//dsy_adc_start();

	for(uint16_t i = 0; i < LED_LAST; i++) 
	{
		dsy_led_driver_set_led(i, 0.0f);
	}
	while(1) 
	{
		dsy_tim_delay_ms(20);
		for(uint16_t i = 0; i < LED_LAST; i++)
		{
			dsy_led_driver_set_led(i, param_bright.value());
		}
	}
}
//static float mtof(float m)
//{
//	return powf(2, (m - 69.0f) / 12.0f) * 440.0f;
//}

