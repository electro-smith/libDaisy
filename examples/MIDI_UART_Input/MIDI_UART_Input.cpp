/** Example of setting reading MIDI Input via UART
 *
 *  This can be used with any 5-pin DIN or TRS connector that has been wired up
 *  to one of the UART Rx pins on Daisy.
 *  This will use D14 as the UART 1 Rx pin
 *
 *  This example will also log incoming messages to the serial port for general MIDI troubleshooting
 *
 *  This includes a demonstration of managing System Realtime messages immediately via
 *  an optional callback. This can be disabled by setting, `kUseRealtimeCallback` to false.
 *  in which case all messages while run through the typical MIDI event queue.
 */
#include "daisy_seed.h"

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

/** Select whether to use the optional system realtime callback or not. */
constexpr const bool kUseRealtimeCallback = true;

/** Indicates from where the MIDI Message was logged */
enum class Source {
    Queue,
    RealtimeCb,
    Unknown,
};

/** Wrapper type that adds a timestamp and source to the MIDI event. */
struct SourcedMidiEvent {
    MidiEvent message;
    Source src;
    uint32_t timestamp;

    SourcedMidiEvent(MidiEvent e, Source s) {
        message = e;
        src = s;
        timestamp = System::GetNow();
    }

    SourcedMidiEvent() : src(Source::Unknown), timestamp(0xffffffff) {}
};

/** Global Hardware access */
DaisySeed       hw;
MidiUartHandler midi;

/** FIFO to hold messages as we're ready to print them */
FIFO<SourcedMidiEvent, 128> event_log;


/** Helper for getting human readable strings from realtime messages */
const char* GetStringForRealTimeType(SystemRealTimeType type) {
    switch(type) {
        case SystemRealTimeType::TimingClock:
            return "Timing Clock";
        case SystemRealTimeType::SRTUndefined0:
            return "SRT Undefined0";
        case SystemRealTimeType::Start:
            return "Start";
        case SystemRealTimeType::Continue:
            return "Continue";
        case SystemRealTimeType::Stop:
            return "Stop";
        case SystemRealTimeType::SRTUndefined1:
            return "SRT Undefined1";
        case SystemRealTimeType::ActiveSensing:
            return "ActiveSensing";
        case SystemRealTimeType::Reset:
            return "Reset";
        default: return "Unknown Type...";
    }
}

/** Helper for getting a string for the source */
const char* GetStringForSource(Source src) {
    switch(src) {
        case Source::Queue: return "Queue";
        case Source::RealtimeCb: return "Realtime Callback";
        case Source::Unknown:
        default: return "Unknown";
    }
}

/**  Optional callback for synchronous realtime message handling
 *
 *   Typically, you would handle your timing-sensitive behavior
 *   right here to take advantage of the syncronous handling.
 *   In this case, we're just going to forward this to the log
 *   to make sure no messages are missed.
*/
void MidiRealtimeCallback(MidiEvent& event) {
    auto ev = SourcedMidiEvent(event, Source::RealtimeCb);
    event_log.PushBack(ev);
}

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    hw.StartLog();

    MidiUartHandler::Config midi_config;
    midi.Init(midi_config);

    if (kUseRealtimeCallback) {
        midi.StartReceiveRt(MidiRealtimeCallback);
    } else {
        midi.StartReceive();
    }

    uint32_t now      = System::GetNow();
    uint32_t log_time = System::GetNow();

    /** Infinite Loop */
    while(1)
    {
        now = System::GetNow();

        /** Monitor MIDI status, and trigger recovery of MIDI Rx if necessary */
        midi.Listen();

        /** Loop through any MIDI Events in the queue */
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

            /** Regardless of message, let's add the message data to our queue to output to the log */
            auto ev = SourcedMidiEvent(msg, Source::Queue);
            event_log.PushBack(ev);
        }

        /** Now separately, every 5ms we'll print the top message in our queue if there is one */
        if(now - log_time > 5)
        {
            log_time = now;
            if(!event_log.IsEmpty())
            {
                auto event = event_log.PopFront();
                auto msg = event.message;
                char outstr[128];
                const char* type_str = MidiEvent::GetTypeAsString(msg);
                if (msg.type == MidiMessageType::SystemRealTime) {
                    const char* src_str = GetStringForSource(event.src);
                    const char* desc_str = GetStringForRealTimeType(msg.srt_type);
                    sprintf(outstr,
                            "time:\t%ld\ttype: %s - %s\tsource: %s\n",
                            event.timestamp,
                            type_str,
                            desc_str,
                            src_str);
                } else {
                    // Only realtime messages are capable of hitting the realtime
                    // callback so we do not need to include the source here.
                    sprintf(outstr,
                            "time:\t%ld\ttype: %s\tChannel:  %d\tData MSB: "
                            "%d\tData LSB: %d\n",
                            event.timestamp,
                            type_str,
                            msg.channel,
                            msg.data[0],
                            msg.data[1]);
                }
                hw.PrintLine(outstr);
            }
        }
    }
}
