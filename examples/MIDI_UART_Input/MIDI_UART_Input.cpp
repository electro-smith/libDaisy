/** Example of setting reading MIDI Input via UART
 *
 *
 *  This can be used with any 5-pin DIN or TRS connector that has been wired up
 *  to one of the UART Rx pins on Daisy.
 *  This will use D14 as the UART 1 Rx pin
 *
 *  This example will also log incoming messages to the serial port for general MIDI troubleshooting
 */
#include "daisy_seed.h"

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

/** Global Hardware access */
DaisySeed       hw;
MidiUartHandler midi;

/** FIFO to hold messages as we're ready to print them */
FIFO<MidiEvent, 128> event_log;

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    hw.StartLog();

    MidiUartHandler::Config midi_config;
    midi.Init(midi_config);

    midi.StartReceive();

    uint32_t now      = System::GetNow();
    uint32_t log_time = System::GetNow();

    /** Infinite Loop */
    while(1)
    {
        now = System::GetNow();

        /** Process MIDI in the background */
        midi.Listen();

        /** Loop through any MIDI Events */
        while(midi.HasEvents())
        {
            MidiEvent msg = midi.PopEvent();

            /** Handle messages as they come in
             *  See DaisyExamples for some examples of this
             */
            switch(msg.type)
            {
                case NoteOn:
                    // Do something on Note On events
                    {
                        uint8_t bytes[3] = {0x90, 0x00, 0x00};
                        bytes[1] = msg.data[0];
                        bytes[2] = msg.data[1];
                        midi.SendMessage(bytes, 3);
                    }
                    break;
                default: break;
            }

            /** Regardless of message, let's add the message data to our queue to output */
            event_log.PushBack(msg);
        }

        /** Now separately, every 5ms we'll print the top message in our queue if there is one */
        if(now - log_time > 5)
        {
            log_time = now;
            if(!event_log.IsEmpty())
            {
                auto msg = event_log.PopFront();
                char outstr[128];
                const char* type_str = MidiEvent::GetTypeAsString(msg);
                sprintf(outstr,
                        "time:\t%ld\ttype: %s\tChannel:  %d\tData MSB: "
                        "%d\tData LSB: %d\n",
                        now,
                        type_str,
                        msg.channel,
                        msg.data[0],
                        msg.data[1]);
                hw.PrintLine(outstr);
            }
        }
    }
}
