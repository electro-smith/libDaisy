#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisy;
using namespace daisysp;

static daisy_handle seed;

Oscillator osc;
svf filt;

static void audioCallback(float *in, float *out, size_t size)
{
    float sig;

    for (size_t i = 0; i < size; i += 2)
    {
        sig = osc.Process();

        filt.process(sig);

        // left out
        out[i] = filt.low();

        // right out
        out[i + 1] = filt.high();
    }
}

int main(void)
{
    // initialize seed hardware and svf daisysp module
    daisy_seed_init(&seed);
    // Initialize Oscillator, and set parameters.
    osc.Init(DSY_AUDIO_SAMPLE_RATE);
    osc.SetWaveform(osc.WAVE_POLYBLEP_SAW);
    osc.SetFreq(250.0);
    osc.SetAmp(0.5);
    // Initialize Filter, and set parameters.
    filt.init(DSY_AUDIO_SAMPLE_RATE);
    filt.set_freq(500.0);
    filt.set_res(0.85);
    filt.set_drive(0.8);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    dsy_adc_start();

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
