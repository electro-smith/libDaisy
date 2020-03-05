#include "daisy_patch.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

#define LEFT (i)
#define RIGHT (i+1)

#define NUM_VOICES 32
#define MAX_DELAY ((size_t)(10.0f * SAMPLE_RATE))

// Simple Gate In -- to be moved to a libdaisy class soon.
struct GateIn
{
    void Init(dsy_gpio *gatepin)
    {
        pin        = gatepin;
        prev_state = 0;
    }

    // Call this periodically, it checks if
    bool Trig()
    {
        bool new_state, out;
        new_state  = !dsy_gpio_read(pin); // inverted because of hardware
        out        = new_state && !prev_state;
        prev_state = new_state;
        return out;
    }

    dsy_gpio *pin;
    uint8_t   prev_state;
};

// Synthesis w/ pseudo-polyphony and decay control.
// Notes are set with MIDI note numbers.
struct PolyPluck
{
    void Init()
    {
        active_voice = 0;
        p_damp       = 0.95f;
        p_decay      = 0.75f;
        for(int i = 0; i < NUM_VOICES; i++)
        {
            plk[i].Init(SAMPLE_RATE, plkbuff[i], 256, PLUCK_MODE_RECURSIVE);
            plk[i].SetDamp(0.85f);
            plk[i].SetAmp(0.18f);
            plk[i].SetDecay(0.85f);
        }
        blk.Init(SAMPLE_RATE);
    }
    float Process(float trig, float note)
    {
        float sig, tval;
        sig = 0.0f;
        if(trig > 0.0f)
        {
            // increment active voice
            active_voice = (active_voice + 1) % NUM_VOICES;
            // set new voice to new note
            plk[active_voice].SetDamp(p_damp);
            plk[active_voice].SetDecay(p_decay);
            plk[active_voice].SetAmp(0.25f);
        }
		plk[active_voice].SetFreq(mtof(note));

        for(int i = 0; i < NUM_VOICES; i++)
        {
            tval = (trig > 0.0f && i == active_voice) ? 1.0f : 0.0f;
            sig += plk[i].Process(tval);
        }
        return blk.Process(sig);
    }
    void SetDecay(float p) { p_damp = p; }
	// Member Variables
    DcBlock blk;
    Pluck  plk[NUM_VOICES];
    float  plkbuff[NUM_VOICES][256];
    float  p_damp, p_decay;
    size_t active_voice;
};

// Hardware
daisy_patch hw;
GateIn      trig_in;

// Synthesis
PolyPluck synth;
// 10 second delay line on the external SDRAM
DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delay;
ReverbSc  verb;

// Persistent filtered value for smooth delay time changes.
float smooth_time;

void AudioCallback(float *in, float *out, size_t size)
{
    float sig, delsig; // Mono Audio Vars
    float trig, nn, decay; // Pluck Vars
    float deltime, delfb, kval; // Delay Vars
    float dry, send, wetl, wetr; // Effects Vars

	// Handle Triggering the Plucks
    trig = 0.0f;
    hw.button1.Debounce();
    if(hw.button1.RisingEdge() || trig_in.Trig())
        trig = 1.0f;

	// Set MIDI Note for new Pluck notes.
    nn = 24.0f + hw.knob1.Process() * 60.0f;
    nn += hw.cv3.Process() * 60.0f;
    nn = static_cast<int32_t>(nn); // Quantize to semitones

    // Read knobs for decay;
    decay = 0.5f + (hw.knob2.Process() * 0.5f);
    synth.SetDecay(decay);

    // Get Delay Parameters from knobs.
    kval    = hw.knob3.Process();
    deltime = (0.001f + (kval*kval) * 5.0f) * SAMPLE_RATE;
    delfb   = hw.knob4.Process();

    // Synthesis.
    for(size_t i = 0; i < size; i += 2)
    {
		// Smooth delaytime, and set.
        fonepole(smooth_time, deltime, 0.0005f);
        delay.SetDelay(smooth_time);

		// Synthesize Plucks
        sig        = synth.Process(trig, nn);
        if(trig)
            trig = 0.0f; // Clear trig to avoid multiple trigs.

		// Handle Delay
		delsig     = delay.Read();
        delay.Write(sig + (delsig * delfb));

		// Create Reverb Send
        dry        = sig + delsig;
        send       = dry * 0.6f;
        verb.Process(send,send, &wetl, &wetr);

		// Output 
        out[LEFT]     = dry + wetl;
        out[RIGHT] = dry + wetr;
    }
}

int main(void)
{
	// Init everything.
    hw.Init();
    trig_in.Init(&hw.gate_in1);
    synth.Init();
    delay.Init();
    delay.SetDelay(SAMPLE_RATE * 0.8f); // half second delay
    verb.Init(SAMPLE_RATE);
    verb.SetFeedback(0.85f);
    verb.SetLpFreq(2000.0f);
    // Start the ADC and Audio Peripherals on the Hardware
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    for(;;) {}
}
