#include "daisysp.h"
#include "daisy_seed.h"

// Shortening long macro for sample rate
#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE
#endif

// Interleaved audio definitions
#define LEFT (i)
#define RIGHT (i+1)

using namespace daisysp;

static daisy_handle seed;

static Compressor comp;
// Helper Modules
static adenv env;
static oscillator osc_a, osc_b;
static metro tick;


static void audioCallback(float *in, float *out, size_t size)
{
	float osc_a_out, osc_b_out, env_out, sig_out;
    for (size_t i = 0; i < size; i += 2)
    {
        // When the metro ticks: 
        // trigger the envelope to start
        if (tick.process())
        {
            env.trigger();
        }

        // Use envelope to control the amplitude of the oscillator.
        env_out = env.process();
        osc_a.set_amp(env_out);
    	osc_a_out = osc_a.process();
        osc_b_out = osc_b.process();
        // Compress the steady tone with the enveloped tone.
        sig_out = comp.Process(osc_b_out, osc_a_out);

        // Output
        out[LEFT] = sig_out; // compressed
        out[RIGHT] = osc_a_out; // key signal
    }
}

int main(void)
{
    // initialize seed hardware and daisysp modules
    daisy_seed_init(&seed);
    comp.Init(SAMPLE_RATE);
    env.init(SAMPLE_RATE);
    osc_a.init(SAMPLE_RATE);
    osc_b.init(SAMPLE_RATE);

    // Set up metro to pulse every second
    tick.init(1.0f, SAMPLE_RATE);    

    // set compressor parameters
    comp.SetThreshold(-64.0f);
    comp.SetRatio(2.0f);
    comp.SetAttack(0.005f);
    comp.SetRelease(0.1250);

    // set adenv parameters
    env.set_time(ADENV_SEG_ATTACK, 0.001);
    env.set_time(ADENV_SEG_DECAY, 0.50);
    env.set_min(0.0);
    env.set_max(0.25);
    env.set_curve_scalar(0); // linear

    // Set parameters for oscillator
    osc_a.set_waveform(oscillator::WAVE_TRI);
    osc_a.set_freq(110);
    osc_a.set_amp(0.25);
    osc_b.set_waveform(oscillator::WAVE_TRI);
    osc_b.set_freq(220);
    osc_b.set_amp(0.25);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
