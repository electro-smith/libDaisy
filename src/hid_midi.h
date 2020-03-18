// # MidiHandler
// ## Description
// Simple MIDI Handler
//
// Parses bytes from an input into valid MidiEvents.
//
// The MidiEvents fill a FIFO queue that the user can pop messages from.
//
// ## Credit
// *author*: shensley
// *date added*: March 2020
#pragma once
#ifndef DSY_MIDI_H
#define DSY_MIDI_H

#include <stdint.h>
#include <stdlib.h>
#include "per_uart.h"
#include "util_ringbuffer.h"

namespace daisy
{
// ## Data
// ### MidiMessageType
// Parsed from the Status Byte, these are the common Midi Messages
// that can be handled.
// At this time only 3-byte messages are correctly parsed into MidiEvents.
// ~~~~
enum MidiMessageType
{
    NoteOff,
    NoteOn,
    PolyphonicKeyPressure,
    ControlChange,
    ProgramChange,
    ChannelPressure,
    PitchBend,
    MessageLast, // maybe change name to MessageUnsupported
};
// ~~~~

// ### NoteOnEvent
// Struct containing note, and velocity data for a given channel.
//
// Can be made from MidiEvent
// ~~~~
struct NoteOnEvent
{
    int     channel;
    uint8_t note;
    uint8_t velocity;
};
// ~~~~
// ### ControlChangeEvent
// Struct containing control number, and value for a given channel.
//
// Can be made from MidiEvent
// ~~~~
struct ControlChangeEvent
{
    int     channel;
    uint8_t control_number;
    uint8_t value;
};
// ~~~~

// ### MidiEvent
// Simple MidiEvent with message type, channel, and data[2] members.
struct MidiEvent
{
    // Newer ish.
    MidiMessageType type;
    int             channel;
    uint8_t         data[2];
	// #### AsNoteOn
    // Returns the data within the MidiEvent as a NoteOnEvent struct.
	// ~~~~
    NoteOnEvent     AsNoteOn()
	// ~~~~
    {
        NoteOnEvent m;
        m.channel  = channel;
        m.note     = data[0];
        m.velocity = data[1];
        return m;
    }

	// #### AsNoteOn
    // Returns the data within the MidiEvent as a NoteOnEvent struct.
	// ~~~~
    ControlChangeEvent AsControlChange()
    // ~~~~
    {
        ControlChangeEvent m;
        m.channel = channel;
        m.control_number = data[0];
        m.value          = data[1];
        return m;
    }
};

// ## MidiHandler
class MidiHandler
{
  public:
    MidiHandler() {}
    ~MidiHandler() {}
    // ### Midi IO Modes
    // Input and Output can be configured separately
    // Multiple Input modes can be selected by OR'ing the values.
	// ~~~~
    enum MidiInputMode
    {
        INPUT_MODE_NONE    = 0x00,
        INPUT_MODE_UART1   = 0x01,
        INPUT_MODE_USB_INT = 0x02,
        INPUT_MODE_USB_EXT = 0x04,
    };
    enum MidiOutputMode
    {
        OUTPUT_MODE_NONE    = 0x00,
        OUTPUT_MODE_UART1   = 0x01,
        OUTPUT_MODE_USB_INT = 0x02,
        OUTPUT_MODE_USB_EXT = 0x04,
    };
    // ~~~~

    // ## Functions

    // ### Init
    // Initializes the MidiHandler
    // ~~~~
    void Init(MidiInputMode in_mode, MidiOutputMode out_mode);
    // ~~~~

    // ### StartReceive
    // Starts listening on the selected input mode(s).
    // MidiEvent Queue will begin to fill, and can be checked with
    void StartReceive();

    void Listen();

    // ### Parse
    // Feed in bytes to state machine from a queue.
    //
    // Populates internal FIFO queue with MIDI Messages
    //
    // For example with uart:
    // midi.Parse(uart.PopRx());
    // ~~~~
    void Parse(uint8_t byte);
    // ~~~~

    // ### HasEvents
    // Checks if there are unhandled messages in the queue
    // ~~~~
    bool HasEvents() const { return event_q_.readable(); }
    // ~~~~

    // ### PopEvent
    //
    // Pops the oldest unhandled MidiEvent from the internal queue
    //
    //
    MidiEvent PopEvent() { return event_q_.Read(); }

  private:
    enum ParserState
    {
        ParserEmpty,
        ParserHasStatus,
        ParserHasData0,
    };
    MidiInputMode            in_mode_;
    MidiOutputMode           out_mode_;
    UartHandler              uart_;
    ParserState              pstate_;
    MidiEvent                incoming_message_;
    RingBuffer<MidiEvent, 8> event_q_;
};

} // namespace daisy

#endif
