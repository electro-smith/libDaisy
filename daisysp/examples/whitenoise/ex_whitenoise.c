#include "daisysp.h"
#include "dsy_seed.h"

static daisy_handle seed;
static dsy_whitenoise whitenoise_gen;
 
static void audioCallback(float *in, float *out, size_t size)
{
    float whitenoise_out;

    for (size_t i = 0; i < size; i += 2)
    {
        whitenoise_out = dsy_whitenoise_process(&whitenoise_gen);

        // left out
        out[i] = whitenoise_out;

        // right out
        out[i + 1] = whitenoise_out;
    }
}

int main(void)
{
    // initialize seed hardware and whitenoise daisysp module
    daisy_seed_init(&seed);
    dsy_whitenoise_init(&whitenoise_gen);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
