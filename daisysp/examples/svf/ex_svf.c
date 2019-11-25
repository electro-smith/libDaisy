#include "daisysp.h"
#include "brd_seed.h"

static daisy_handle seed;
static dsy_svf filter;

static void audioCallback(float *in, float *out, size_t size)
{
    float inL;

    for (size_t i = 0; i < size; i += 2)
    {
        inL = in[i];

        dsy_svf_process(&filter, &inL);

        // left out
        out[i] = dsy_svf_low(&filter);

        // right out
        out[i + 1] = dsy_svf_high(&filter);
    }
}

int main(void)
{
    // initialize seed hardware and svf daisysp module
    daisy_seed_init(&seed);
    dsy_svf_init(&filter, DSY_AUDIO_SAMPLE_RATE);

    dsy_svf_set_fc(&filter, 5000); // cutoff frequency
    dsy_svf_set_res(&filter, 0.3); // resonance
    dsy_svf_set_drive(&filter, 0); // drive

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    dsy_adc_start();

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
