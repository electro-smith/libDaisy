#include "libdaisy.h"
#include "daisysp.h"
#include "dsy_patch_bsp.h"
#include "dsy_seed.h"

// create daisy handle with variable name "seed"
static daisy_handle seed;

// create struct of type dsy_whitenoise_t with variable name "whitenoise_gen"
static dsy_whitenoise whitenoise_gen;
 
static void audioCallback(float *in, float *out, size_t size)
{
    for (size_t i = 0; i < size; i += 2)
    {
        float whitenoise_out;

        // call whitenoise process function, pass in pointer to whitenoise module called whitenoise_gen and return value to whitenoise_out
        whitenoise_out = dsy_whitenoise_process(&whitenoise_gen);

        // send whitenoise signal to audio output buffer, L channel
        out[i] = whitenoise_out;

        // send whitenoise signal to audio output buffer, R channel
        out[i + 1] = whitenoise_out;
    }
}

int main(void)
{
    // initialize daisy seed by passing a pointer to struct of type daisy_handle named "seed"
    daisy_seed_init(&seed);

    // initialize whitenoise module by passing a pointer to a stuct of type dsy_whitenoise_t named "whitenoise_gen"
    dsy_whitenoise_init(&whitenoise_gen);

    // create callback using internal audio peripherals, and attach to audioCallback function
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    // loop forever...
    while(1) {}
}
