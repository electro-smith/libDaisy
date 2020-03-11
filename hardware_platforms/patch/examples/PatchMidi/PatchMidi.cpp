#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;

daisy_field         hw;
UartHandler         uart;
MidiHandler         midi;
uint8_t             mybuff[16];
daisysp::Oscillator osc;
daisysp::Svf        filt;
AnalogControl       knobs[4];

void AudioCallback(float *in, float *out, size_t size)
{
    float sig;
    for(size_t i = 0; i < 4; i++)
    {
        knobs[i].Process();
    }
    for(size_t i = 0; i < size; i += 2)
    {
        sig = osc.Process();
        filt.Process(sig);
        out[i] = out[i + 1] = filt.Low();
    }
}


int main(void)
{
    // Init
    //    hw.Init();
    daisy_field_init(&hw);
    uart.Init();
    midi.Init();

    // Synthesis
    osc.Init(SAMPLE_RATE);
    osc.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);
    filt.Init(SAMPLE_RATE);

    // Start stuff.
    uart.StartRx(mybuff, 3);
    // This stuff needs to have an interface in daisy_field.h
    dsy_adc_start();
    size_t blocksize = 12;
    dsy_audio_set_blocksize(DSY_AUDIO_INTERNAL, blocksize);
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, AudioCallback);
    dsy_audio_start(DSY_AUDIO_INTERNAL);
    for(int i = 0; i < 4; i++)
    {
        knobs[i].Init(dsy_adc_get_mux_rawptr(0, i), SAMPLE_RATE / blocksize);
    }

    //    hw.StartAdc();
    //    hw.StartAudio(AudioCallback);
    uint32_t last_send, now;
    now = last_send = dsy_system_getnow();
    for(;;)
    {
        // Until a mechanism is added, we'll just test for sameness..
        while(uart.Readable())
        {
            midi.Parse(uart.PopRx());
        }
        while(midi.HasEvents())
        {
            MidiEvent m;
            m = midi.PopEvent();
            switch(m.type)
            {
                case NoteOn:
                    if(m.data[1] != 0)
                    {
                        osc.SetFreq(daisysp::mtof(m.data[0]));
                        osc.SetAmp((m.data[1] / 127.0f));
                    }
                    break;
                case ControlChange:
                    switch(m.data[0])
                    {
                        case 1:
                            // CC 1 for cutoff.
                            filt.SetFreq(daisysp::mtof((float)m.data[1]));
                            break;
                        case 2:
                            // CC 2 for res.
                            filt.SetRes(((float)m.data[1] / 127.0f));
                            break;

                        default: break;
                    }
                    break;
                default: break;
            }
        }
        // Write MIDI Out for Knobs..
        // 4 knobs do CC1-4 0-127
//        now = dsy_system_getnow();
//        if(now - last_send > 20)
//        {
//            last_send = now;
//            for(size_t i = 0; i < 4; i++)
//            {
//                uint8_t outval, ccnum, sb;
//                uint8_t buff[3];
//                outval  = static_cast<uint8_t>(knobs[i].Value() * 127.0f);
//                sb      = 0xb0;
//                ccnum   = i + 1;
//                buff[0] = sb;
//                buff[1] = ccnum;
//                buff[2] = outval;
//                uart.PollTx(buff, 3);
//            }
//        }
    }
}
