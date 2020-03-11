#include "daisy_patch.h"
#include "daisysp.h"

using namespace daisy;

struct MidiMessage
{
    void Set(uint8_t *bytes)
    {
        sb  = bytes[0];
        db0 = bytes[1];
        db1 = bytes[2];
    }
    bool Equals(const MidiMessage &m)
    {
        return (sb == m.sb) && (db0 == m.db0) && (db1 == m.db1);
    }
    uint8_t sb, db0, db1;
};

daisy_patch hw;
UartHandler uart;
MidiHandler midi;
uint8_t     mybuff[16];

uint8_t mnote, mvel;
uint8_t mcc, mval;

daisysp::Oscillator osc;
void                AudioCallback(float *in, float *out, size_t size)
{
    float sig;
    osc.SetAmp((mvel / 127.0f) * 0.7f);
    osc.SetFreq(daisysp::mtof(mnote));
    for(size_t i = 0; i < size; i += 2)
    {
        sig    = osc.Process();
        out[i] = out[i + 1] = sig;
    }
}

int main(void)
{
    // Local vars
    MidiMessage cur, prev;
    MidiEvent   event;
    // Init
    hw.Init();
    uart.Init();
    midi.Init();
    // Synthesis
    osc.Init(SAMPLE_RATE);
    osc.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);

    // Start stuff.
    uart.Receive(mybuff, 3);
    hw.StartAudio(AudioCallback);
    for(;;)
    {
        // Until a mechanism is added, we'll just test for sameness..
        prev.sb  = cur.sb;
        prev.db0 = cur.db0;
        prev.db1 = cur.db1;
        cur.Set(mybuff);
        if(!cur.Equals(prev))
        {
            // Probably change mybuff to handle 'midimessage'
            event = midi.Parse(mybuff, 3);
            if(event.is_note()) {
                mnote = event.note_;
                mvel  = event.vel_;
            }
            if(event.is_cc())
            {
                mcc = event.cc_;
                mval = event.val_;
            }
        }
    }
}
