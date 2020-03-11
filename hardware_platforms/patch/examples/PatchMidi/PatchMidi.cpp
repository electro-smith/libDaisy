#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;

daisy_field         hw;
MidiHandler         midi;
daisysp::Oscillator osc;
daisysp::Svf        filt;
AnalogControl       knobs[4];
float               filtval, midifiltval;

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
    midi.Init(MidiHandler::INPUT_MODE_UART1, MidiHandler::OUTPUT_MODE_UART1);

    // Synthesis
    osc.Init(SAMPLE_RATE);
    osc.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);
    filt.Init(SAMPLE_RATE);

    // Start stuff.
    midi.StartReceive();

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
    int curled      = 0;

    for(;;)
    {
        midi.Listen();
        // Handle MIDI Events
        while(midi.HasEvents())
        {
            MidiEvent m;
            m = midi.PopEvent();
            switch(m.type)
            {
                case NoteOn:
                {
                    NoteOnEvent p;
                    if(m.data[1]
                       != 0) // This is to avoid Max/MSP Note outs for now..
                    {
                        p = m.AsNoteOn();
                        osc.SetFreq(daisysp::mtof(p.note));
                        osc.SetAmp((p.velocity / 127.0f));
                    }
                }
                break;
                case ControlChange:
                {
                    ControlChangeEvent p;
                    switch(p.control_number)
                    {
                        case 1:
                            // CC 1 for cutoff.
                            midifiltval = (daisysp::mtof((float)p.value));
                            break;
                        case 2:
                            // CC 2 for res.
                            filt.SetRes(((float)p.value / 127.0f));
                            break;
                        default: break;
                    }
                    break;
                }
                default: break;
            }
        }

        // Handle Synthesis settings.
        filtval = midifiltval + daisysp::mtof(knobs[0].Value() * 127.0f);
        if(filtval > 14000.0f)
        {
            filtval = 14000.0f;
        }
        filt.SetFreq(filtval);
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
