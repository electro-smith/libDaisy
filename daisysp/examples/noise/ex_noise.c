#include "libdaisy.h"
#include "daisysp.h"
#include "dsy_seed.h"

static daisy_handle seed;
static dsy_noise_t noise_gen;
 
static void audioCallback(float *in, float *out, size_t size)
{
    float noise_out;

    for (size_t i = 0; i < size; i += 2)
    {
        noise_out = dsy_noise_process(&noise_gen);

        // left out
        out[i] = noise_out;

        // right out
        out[i + 1] = noise_out;
    }
}

int main(void)
{
    // initialize seed hardware and noise daisysp module
    daisy_seed_init(&seed);
    dsy_noise_init(&noise_gen);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
