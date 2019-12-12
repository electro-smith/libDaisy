#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;

static daisy_handle seed;
static adenv env;
static oscillator osc;

static void audioCallback(float *in, float *out, size_t size)
{
	float osc_out, env_out;
    uint8_t current_segment;
    for (size_t i = 0; i < size; i += 2)
    {
        current_segment = env.current_segment();
        if (current_segment == ADENV_SEG_IDLE) 
        {
            env.trigger();
        }

        env_out = env.process();
        osc.set_freq(env_out);
    	osc_out = osc.process();

    	// left out
        out[i] = osc_out;

        // right out
        out[i+1] = osc_out;
    }
}

int main(void)
{
	// initialize seed hardware and daisysp modules
    daisy_seed_init(&seed);
    env.init(DSY_AUDIO_SAMPLE_RATE);
    osc.init(DSY_AUDIO_SAMPLE_RATE);

    // set adenv parameters
    env.set_time(ADENV_SEG_ATTACK, 1);
    env.set_time(ADENV_SEG_DECAY, 1);
    env.set_min(1);
    env.set_max(1000);
    env.set_curve_scalar(0); // linear

    // Set parameters for oscillator
    osc.set_waveform(osc.WAVE_POLYBLEP_TRI);
    osc.set_freq(1);
    osc.set_amp(0.5);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
