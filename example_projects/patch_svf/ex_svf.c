#include "libdaisy.h"
#include "daisysp.h"
#include "dsy_patch_bsp.h"

static daisy_patch patch;
static dsy_svf_t filt;

static void audioCallback(float *in, float *out, size_t size)
{
    float cutoff, resonance, in_amp, inL;

    // read controls at k rate
    cutoff = dsy_adc_get_float(DSY_PATCH_KNOB_1)*10000;
    resonance = dsy_adc_get_float(DSY_PATCH_KNOB_2);
    in_amp = dsy_adc_get_float(DSY_PATCH_KNOB_3);

    // set filter module params with control values
    dsy_svf_set_fc(&filt, cutoff);
    dsy_svf_set_res(&filt, resonance);

    for (size_t i = 0; i < size; i += 2)
    {	
    	// scale input signal by amp knob
    	inL = in[i] * in_amp;

    	// send input to svf module
    	dsy_svf_process(&filt, &inL);

    	// send LPF to left output
        out[i] = dsy_svf_low(&filt);

        // send HPF to right output
        out[i+1] = dsy_svf_high(&filt);
    }
}

int main(void)
{
    // initialize seed/patch hardware and daisysp modules
    daisy_seed_init(&patch.seed);
    daisy_patch_init(&patch);
    dsy_svf_init(&filt, DSY_AUDIO_SAMPLE_RATE);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start controls ADC
    dsy_adc_start();

    // start audio callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
