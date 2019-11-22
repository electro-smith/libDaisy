#include "libdaisy.h"
#include "daisysp.h"
#include "dsy_patch_bsp.h"
#include "dsy_seed.h"

// create daisy handle with variable name "seed"
static daisy_handle seed;

// create struct of type dsy_noise_t with variable name "noise_gen"
static dsy_noise_t noise_gen;
 
static void audioCallback(float *in, float *out, size_t size)
{
    for (size_t i = 0; i < size; i += 2)
    {
        float noise_out;

        // call noise process function, pass in pointer to noise module called noise_gen and return value to noise_out
        noise_out = dsy_noise_process(&noise_gen);

        // send noise signal to audio output buffer, L channel
        out[i] = noise_out;

        // send noise signal to audio output buffer, R channel
        out[i + 1] = noise_out;
    }
}

int main(void)
{
    // initialize daisy seed by passing a pointer to struct of type daisy_handle named "seed"
    daisy_seed_init(&seed);

    // initialize noise module by passing a pointer to a stuct of type dsy_noise_t named "noise_gen"
    dsy_noise_init(&noise_gen);

    // create callback using internal audio peripherals, and attach to audioCallback function
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    // loop forever...
    while(1) {}
}
