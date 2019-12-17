#include "daisysp.h"
#include "daisy_seed.h"

// Shortening long macro for sample rate
#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE
#endif

// Interleaved audio definitions
#define LEFT (i)
#define RIGHT (i+1)

// Set max delay time to 0.75 of samplerate.
#define MAX_DELAY static_cast<size_t>(SAMPLE_RATE * 0.75f)

using namespace daisysp;

static daisy_handle seed;

// Helper Modules
static adenv env;
static oscillator osc;
static metro tick;

// Declare a delayline of MAX_DELAY number of floats.
static delayline<float, MAX_DELAY> del;

static void audioCallback(float *in, float *out, size_t size)
{
	float osc_out, env_out, feedback, del_out, sig_out;
    for (size_t i = 0; i < size; i += 2)
    {
        // When the metro ticks: 
        // trigger the envelope to start, and change freq of oscillator.
        if (tick.process())
        {
            float freq = rand() % 200;
            osc.set_freq(freq + 100.0f);
            env.trigger();
        }

        // Use envelope to control the amplitude of the oscillator.
        env_out = env.process();
        osc.set_amp(env_out);
    	osc_out = osc.process();

        // Read from delay line
        del_out = del.read();
        // Calculate output and feedback
        sig_out = del_out + osc_out;
        feedback = (del_out * 0.75f) + osc_out;

        // Write to the delay
        del.write(feedback);

        // Output
        out[LEFT] = sig_out;
        out[RIGHT] = sig_out;
    }
}

int main(void)
{
    // initialize seed hardware and daisysp modules
    daisy_seed_init(&seed);
    env.init(SAMPLE_RATE);
    osc.init(SAMPLE_RATE);
    del.init();

    // Set up metro to pulse every second
    tick.init(1.0f, SAMPLE_RATE);    

    // set adenv parameters
    env.set_time(ADENV_SEG_ATTACK, 0.001);
    env.set_time(ADENV_SEG_DECAY, 0.50);
    env.set_min(0.0);
    env.set_max(0.25);
    env.set_curve_scalar(0); // linear

    // Set parameters for oscillator
    osc.set_waveform(osc.WAVE_TRI);
    osc.set_freq(220);
    osc.set_amp(0.25);

    // Set Delay time to 0.75 seconds
    del.set_delay(SAMPLE_RATE * 0.75f);

    // define callback
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, audioCallback);

    // start callback
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}
