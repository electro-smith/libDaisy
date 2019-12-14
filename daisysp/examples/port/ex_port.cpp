#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;

static daisy_handle seed;
static port slew;
static metro clock;
static oscillator osc_sine;

float freq;

static void audioCallback(float *in, float *out, size_t size)
{
    float sine;
    uint8_t tic;
    for (size_t i = 0; i < size; i += 2)
    {   
        tic = clock.process();
        if (tic)
        {
            freq = rand() % 500;
        }

        freq = slew.process(freq);
        osc_sine.set_freq(freq);

        sine = osc_sine.process();

        // left out
        out[i] = sine;

        // right out
        out[i+1] = sine;
    }
}

int main(void)
{
    // initialize seed hardware and daisysp modules
    daisy_seed_init(&seed);

    // set params for port object
    slew.init(DSY_AUDIO_SAMPLE_RATE, .01);

    clock.init(2, DSY_AUDIO_SAMPLE_RATE);

    // set parameters for sine oscillator object
    osc_sine.init(DSY_AUDIO_SAMPLE_RATE);
    osc_sine.set_waveform(oscillator::WAVE_SIN);
    osc_sine.set_freq(100);
    osc_sine.set_amp(0.25);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
