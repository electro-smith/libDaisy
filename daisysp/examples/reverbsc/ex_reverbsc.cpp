#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;

static daisy_handle seed;

reverbsc verb;
 
static void audioCallback(float *in, float *out, size_t size)
{
    for (size_t i = 0; i < size; i += 2)
    {
        verb.process(in[i], in[i+1], &out[i], &out[i+1]);
    }
}

int main(void)
{
    // initialize seed hardware and whitenoise daisysp module
    daisy_seed_init(&seed);
    verb.init(DSY_AUDIO_SAMPLE_RATE);
    verb.set_feedback(0.85f);
    verb.set_lpfreq(18000.0f);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
