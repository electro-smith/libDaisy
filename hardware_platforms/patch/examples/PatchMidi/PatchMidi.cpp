#include "daisy_patch.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

#define LEFT (i)
#define RIGHT (i + 1)

#define NUM_VOICES 32
#define MAX_DELAY ((size_t)(10.0f * SAMPLE_RATE))

// Hardware
daisy_patch hw;
UartHandler uart;
MidiHandler midi;
uint8_t     mybuff[16];

void AudioCallback(float *in, float *out, size_t size)
{
    // Synthesis.
    for(size_t i = 0; i < size; i += 2)
    {
        // Output
        out[LEFT]  = in[LEFT];
        out[RIGHT] = in[RIGHT];
    }
}
int     current_note, current_velocity;
int     current_cc, current_val;
uint8_t midibuff[3];

static uint8_t reverse(uint8_t b)
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}
uint8_t db_idx     = 0;
uint8_t midistatus = 0;


enum MidiState
{
    MidiWaiting,
    MidiStatus,
    MidiData0,
    MidiData1,
    MidiValid,
};

struct midimessage
{
    uint8_t sb, db0, db1;
};

midimessage midihistory[64];
uint8_t     bytehistory[9];

int main(void)
{
    // Init everything.
    hw.Init();
    uart.Init();
    midi.Init();
    // Start the ADC and Audio Peripherals on the Hardware
    //    hw.StartAdc();
    //    hw.StartAudio(AudioCallback);
    MidiEvent event;
    uint8_t   mybyte;
    uint8_t   hist_idx;
    uint8_t   bytehist_idx;
    midistatus  = MidiWaiting;
    midibuff[0] = 0;
    midibuff[1] = 0;
    midibuff[2] = 0;
    for(;;)
    {
        mybuff[0] = 0;
        if(uart.PollReceive(mybuff, 1) == HAL_OK)
        {
            //mybyte = reverse(mybuff[0]);

//            for(uint8_t i = 0; i < 3; i++)
            {
                mybyte                    = (mybuff[0]);
                bytehistory[bytehist_idx] = mybyte;
                bytehist_idx              = (bytehist_idx + 1) % 9;
                switch(midistatus)
                {
                    case MidiWaiting:
                        // Check for status byte:
                        if(mybyte > 127)
                        {
                            midibuff[0] = mybyte;
                            midistatus  = MidiStatus;
                        }
                        break;
                    case MidiStatus:
                        if(mybyte < 127)
                        {
                            midibuff[1] = mybyte;
                            midistatus  = MidiData0;
                        }
                        break;
                    case MidiData0:
                        if(mybyte < 127)
                        {
                            midibuff[2] = mybyte;
                            midistatus  = MidiData1;
                        }
                        break;
                    case MidiData1:
                        event = midi.Parse(midibuff, 3);
                        // Store in history for looking at
                        midimessage hist;
                        hist.sb               = midibuff[0];
                        hist.db0              = midibuff[1];
                        midihistory[hist_idx] = hist;
                        hist_idx              = (hist_idx + 1) % 64;
                        midistatus            = MidiWaiting;
                        break;
                    default: break;
                }
            }
        }
    }
}
