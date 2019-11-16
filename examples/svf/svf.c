#include "libdaisy.h"
#include "daisysp.h"
#include "dsy_patch_bsp.h"
#include "dsy_seed.h"

// create daisy handle with variable name "seed"
static daisy_handle seed;

// create struct of type dsy_svf_t with variable name "filter"
static dsy_svf_t filter;

// audio callback 
static void audioCallback(float *in, float *out, size_t size)
{
    float inL;

    for (size_t i = 0; i < size; i += 2)
    {
        // assign codec left input to inL
        inL = in[i];

        // set parameters of svf
        dsy_svf_set_fc(&filter, 5000); // cutoff frequency
        dsy_svf_set_res(&filter, 0.3); // resonance
        dsy_svf_set_drive(&filter, 0); // drive

        // send inL to input of svf
        dsy_svf_process(&filter, &inL);

        // send LPF output from SVF module to L output on Seed
        out[i] = dsy_svf_low(&filter);

        // send HPF output from SVF module to R output on Seed
        out[i + 1] = dsy_svf_high(&filter);
        
    }
}

int main(void)
{
    // initialize daisy seed by passing a pointer to struct of type daisy_handle named "seed"
    daisy_seed_init(&seed);

    // initialize svf module by passing a pointer to a stuct of type dsy_svf_t named "filter" and setting sample rate to 48kHz
    dsy_svf_init(&filter, 48000);

    // create callback using internal audio peripherals, and attach to audioCallback function
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start codec ADC
    dsy_adc_start();

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    // loop forever...
    while(1) {}
}
