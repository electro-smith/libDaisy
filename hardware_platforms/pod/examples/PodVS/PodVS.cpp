#include "daisy_patch.h"
#include "daisysp.h"
#include <math.h>

#define MYPREFERENCE

using namespace daisy;
using namespace daisysp;

daisy_patch hw;
oscillator osc;
whitenoise	 nse;
static uint8_t wf;
parameter param_freq, param_nse_amp, param_osc_amp, param_bright;

static void audio(float *in, float *out, size_t size)
{
	float sig, freq, namp, oamp;
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
	param_freq.init(hw.ctrl(KNOB_1), 20.0f, 20000.0f, param_freq.CURVE_LOG);
	param_nse_amp.init(hw.ctrl(KNOB_2), 0.0f, 1.0f, param_nse_amp.CURVE_EXP);
	param_osc_amp.init(hw.ctrl(KNOB_3), 0.0f, 0.4f, param_osc_amp.CURVE_LINEAR);
	param_bright.init(hw.ctrl(KNOB_4), 0.0f, 1.0f, param_bright.CURVE_CUBE);
#else
	param_freq.init(hw.knob1, 20.0f, 20000.0f, param_freq.CURVE_LOG);
	param_nse_amp.init(hw.knob2, 0.0f, 1.0f, param_nse_amp.CURVE_EXP);
	param_osc_amp.init(hw.knob3, 0.0f, 0.4f, param_osc_amp.CURVE_LINEAR);
	param_bright.init(hw.knob4, 0.0f, 1.0f, param_bright.CURVE_CUBE);
#endif
	// Init Osc and Nse
	osc.init(SAMPLE_RATE);
	nse.init();

	// Old style still 
	dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audio);
	dsy_audio_start(DSY_AUDIO_INTERNAL);
	dsy_adc_start();

	for(uint16_t i = 0; i < LED_LAST; i++) 
	{
		dsy_led_driver_set_led(i, 0.0f);
	}
	while(1) 
	{
		dsy_system_delay(20);
		for(uint16_t i = 0; i < LED_LAST; i++) 
		{
			dsy_led_driver_set_led(i, param_bright.value());
		}
		
		dsy_led_driver_update();
	}
}
