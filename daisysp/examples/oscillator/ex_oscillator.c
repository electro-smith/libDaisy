#include "daisysp.h"
#include "daisy_seed.h"

static daisy_handle seed;
static dsy_oscillator osc;

static void audioCallback(float *in, float *out, size_t size)
{
	float sig;
    for (size_t i = 0; i < size; i += 2)
    {
    	sig = dsy_oscillator_process(&osc);

    	// left out
        out[i] = sig;

        // right out
        out[i+1] = sig;
    }
}

int main(void)
{
	// initialize seed hardware and oscillator daisysp module
    daisy_seed_init(&seed);
    dsy_oscillator_init(&osc, DSY_AUDIO_SAMPLE_RATE);

    //  create full scale sine wave at 440hz 
	osc.wave = DSY_OSC_WAVE_SIN;
    osc.freq = 440;
    osc.amp = 1;

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
