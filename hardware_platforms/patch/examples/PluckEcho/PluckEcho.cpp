#include "daisy_patch.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

#define NUM_VOICES 16

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

// Synthesis w/ pseudo-polyphony
struct PolyPluck
{
    // Inits NUM_VOICES plucks.
    void Init()
    {
        active_voice = 0;
        p_damp       = 0.95f;
        p_decay      = 0.75f;
        for(int i = 0; i < NUM_VOICES; i++)
        {
            plk[i].Init(SAMPLE_RATE, plkbuff[i], 256, PLUCK_MODE_RECURSIVE);
            plk[i].SetDamp(0.85f);
            plk[i].SetAmp(0.25f);
            plk[i].SetDecay(0.85f);
        }
        blk.Init(SAMPLE_RATE);
    }

    void SetParams(float damp, float decay)
    {
        p_damp  = damp;
        p_decay = decay;
    }
    void SetDecay(float p) { p_damp = p; }
    // Triggers new note with specified midi note
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
    Pluck  plk[NUM_VOICES];
    DcBlock blk;
    float  plkbuff[NUM_VOICES][256];
    float  p_damp, p_decay;
    size_t active_voice;
};

DelayLine<float, (size_t)(SAMPLE_RATE * 10.0f)> DSY_SDRAM_BSS delay;
//DelayLine<float, (size_t)(SAMPLE_RATE * 1.f)> delay;

// Hardware
daisy_patch hw;
GateIn      trig_in;
// Synthesis
PolyPluck synth;
ReverbSc  verb;

float smooth_time;

void AudioCallback(float *in, float *out, size_t size)
{
    float sig, delsig;
    float trig, nn, decay;
    float deltime, delfb, kval;
    float dry, send, wetl, wetr;
    trig = 0.0f;
    sig  = 0.0f;
    hw.button1.Debounce();
    if(hw.button1.RisingEdge() || trig_in.Trig())
    {
        trig = 1.0f;
    }
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
        fonepole(smooth_time, deltime, 0.0005f);
        delay.SetDelay(smooth_time);
        sig        = synth.Process(trig, nn);
		delsig     = delay.Read();
        delay.Write(sig + (delsig * delfb));
        if(trig)
            trig = 0.0f;
        dry        = sig + delsig;
        send       = dry * 0.6f;
        verb.Process(send,send, &wetl, &wetr);
        out[i]     = dry + wetl;
        out[i + 1] = dry + wetr;
    }
}


int main(void)
{
    hw.Init();
    trig_in.Init(&hw.gate_in1);
    synth.Init();
    delay.Init();
    delay.SetDelay(SAMPLE_RATE * 0.8f); // half second delay
    verb.Init(SAMPLE_RATE);
    verb.SetFeedback(0.85f);
    verb.SetLpFreq(2000.0f);
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    for(;;) {}
}
