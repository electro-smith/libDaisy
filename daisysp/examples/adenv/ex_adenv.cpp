#include "daisysp.h"
#include "daisy_seed.h"

// Shortening long macro for sample rate
#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE
#endif

// Interleaved audio definitions
#define LEFT (i)
#define RIGHT (i+1)

//#define LOOP 1

using namespace daisysp;

static daisy_handle seed;
static adenv env;
static oscillator osc;

metro tick;

static void audioCallback(float *in, float *out, size_t size)
{
	float osc_out, env_out;
    uint8_t current_segment;
    for (size_t i = 0; i < size; i += 2)
    {
        // This is your looping mechanism
        // which works fine, but requires a 
        // deeper understanding of the stages, etc.
        /*
        current_segment = env.get_current_segment();
        if (current_segment == ADENV_SEG_IDLE) 
        {
            env.trigger();
        }
        */

#ifdef LOOP
        // Here's a different one: (more typical C++ use of an accessor function)
        if (!env.is_running())
        {
            env.trigger();
        }
#else
        // Here's an easier to see example of using the 
        // envelope with a typical trigger input.
        if (tick.process())
        {
            env.trigger();
        }
#endif
        env_out = env.process();
        osc.set_amp(env_out);
    	osc_out = osc.process();

        out[LEFT] = osc_out;
        out[RIGHT] = osc_out;
    }
}

int main(void)
{
    // initialize seed hardware and daisysp modules
    daisy_seed_init(&seed);
    env.init(SAMPLE_RATE);
    osc.init(SAMPLE_RATE);

    // Set up metro to pulse every second
    tick.init(1.0f, SAMPLE_RATE);    

    // set adenv parameters
    env.set_time(ADENV_SEG_ATTACK, 0.15);
    env.set_time(ADENV_SEG_DECAY, 0.35);
    env.set_min(0.0);
    env.set_max(0.25);
    env.set_curve_scalar(0); // linear

    // Set parameters for oscillator
    osc.set_waveform(osc.WAVE_TRI);
    osc.set_freq(220);
    osc.set_amp(0.25);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
