#include "daisy_field.h"
#include "daisysp.h"
daisy_field hw;
#define NUM_VOICES 16


struct voice
{
    void init()
    {
        osc_.Init(DSY_AUDIO_SAMPLE_RATE);
        amp_ = 0.0f;
        osc_.SetAmp(0.5f);
        osc_.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);
        on_ = false;
    }
    float process()
    {
        float sig;
        amp_ += 0.0025f * ((on_ ? 1.0f : 0.0f) - amp_);
        sig = osc_.Process() * amp_;
        return sig;
    }
    void set_note(float nn) { osc_.SetFreq(daisysp::mtof(nn)); }

    daisysp::Oscillator osc_;
    float               amp_, midibase_;
    bool                on_;
};

//daisysp::Oscillator osc[8];
voice v[NUM_VOICES];


uint8_t buttons[16];

// Use bottom row to set major scale
float scale[16]   = {0.f,
                   2.f,
                   4.f,
                   5.f,
                   7.f,
                   9.f,
                   11.f,
                   12.f,
                   1.f,
                   3.f,
                   0.f,
                   6.f,
                   8.f,
                   10.f,
                   0.0f};
float active_note = scale[0];

int8_t octaves = 0;

static daisysp::reverbsc verb;
// Use two side buttons to change octaves.

void AudioCallback(float *in, float *out, size_t size)
{
    bool trig, use_verb;
    dsy_sr_4021_update(&hw.keyboard_sr);
    hw.switches[SW_1].Debounce();
    hw.switches[SW_2].Debounce();
    hw.switches[SW_3].Debounce();
    if(hw.switches[SW_1].RisingEdge())
    {
        octaves -= 1;
        trig = true;
    }
    if(hw.switches[SW_2].RisingEdge())
    {
        octaves += 1;
        trig = true;
    }
    if(hw.switches[SW_3].Pressed())
    {
        use_verb = true;
    }
    else
    {
		use_verb = false;
    }

    if(octaves < 0) 
            octaves = 0;
    if(octaves > 4)
        octaves = 4;

    if(trig)
    {
        for(int i = 0; i < NUM_VOICES; i++)
        {
            v[i].set_note((12.0f * octaves) + 24.0f + scale[i]);
        }
    }
    for(size_t i = 0; i < 16; i++)
    {
        buttons[i] = dsy_sr_4021_state(&hw.keyboard_sr, i) | (buttons[i] << 1);
        //        if(buttons[i] == 0x80)
        //        {

        //		if(i < 8)
        {
            if(buttons[i] == 0xFF)
            {
                v[i].on_ = false;
            }
            else if(buttons[i] == 0x00)
            {
                v[i].on_ = true;
            }
        }
        //        active_note = i < 8 ? scale[i] : 0.f;
        // osc[i].SetFreq(
        // daisysp::mtof((12.0f * octaves) + 24.0f + active_note));
        //        }
    }
    float sig, send;
    float wetl, wetr;
    for(size_t i = 0; i < size; i += 2)
    {
        sig = 0.0f;
        for(int i = 0; i < NUM_VOICES; i++)
        {
			if (i != 10 && i != 14 && i != 15)
				sig += v[i].process();
        }
        send = sig * 0.35f;
        verb.process(send,send, &wetl, &wetr);
        //        wetl = wetr = sig;
        if(use_verb)
            wetl = wetr = 0.0f;
        out[i] = (sig + wetl) * 0.5f;
        out[i + 1] = (sig + wetr) * 0.5f;
    }
}

int main(void)
{
    daisy_field_init(&hw);
    octaves = 2;
    for(int i = 0; i < NUM_VOICES; i++)
    {
        v[i].init();
        v[i].set_note((12.0f * octaves) + 24.0f + scale[i]);
    }
    verb.init(DSY_AUDIO_SAMPLE_RATE);
    verb.set_feedback(0.94f);
    verb.set_lpfreq(8000.0f);
    //    osc.Init(DSY_AUDIO_SAMPLE_RATE);
    //    osc.SetFreq(100.0f);
    //    osc.SetAmp(0.5f);
    //    osc.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);
    dsy_adc_start();
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, AudioCallback);
    //dsy_audio_set_blocksize(DSY_AUDIO_INTERNAL, 48);
    dsy_audio_set_blocksize(DSY_AUDIO_INTERNAL, 256);
    dsy_audio_start(DSY_AUDIO_INTERNAL);
    for(;;) {}
}
