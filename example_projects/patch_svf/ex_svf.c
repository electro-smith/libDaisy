#include "libdaisy.h"
#include "daisysp.h"
#include "dsy_patch_bsp.h"

static daisy_patch patch;
static dsy_svf_t filt;

static void audioCallback(float *in, float *out, size_t size)
{
    float cutoff, resonance, drive, in_amp;
    for (size_t i = 0; i < size; i += 2)
    {
        // read controls
        cutoff = dsy_adc_get_float(DSY_PATCH_KNOB_1);
        dsy_svf_set_fc(&filt, cutoff);

        resonance = dsy_adc_get_float(DSY_PATCH_KNOB_2);
        dsy_svf_set_res(&filt, resonance);

        in_amp = dsy_adc_get_float(DSY_PATCH_KNOB_3);

        drive = dsy_adc_get_float(DSY_PATCH_KNOB_4);
        dsy_svf_set_drive(&filt, drive);

        out[i] = 

        //out[i] = 
        
        //out[i] = (dryL * drylevel) + wetL;
        //out[i + 1] = (dryR * drylevel) + wetR;
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
