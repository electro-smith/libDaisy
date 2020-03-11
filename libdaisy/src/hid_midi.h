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
#include "util_ringbuffer.h"

namespace daisy 
{
// ## Data
// ### MidiMessageType
// Parsed from the Status Byte, these are the common Midi Messages
// that can be handled.
// At this time only 3-byte messages are correctly parsed into MidiEvents.
enum MidiMessageType
{
// ~~~~
    NoteOff,
    NoteOn,
    PolyphonicKeyPressure,
    ControlChange,
    ProgramChange,
    ChannelPressure,
    PitchBend,
    MessageLast, // maybe change name to MessageUnsupported
// ~~~~
};

// ### MidiEvent
// Simple MidiEvent with message type, channel, and data[2] members.
struct MidiEvent
{
    // Newer ish.
    MidiMessageType type;
    int             channel;
    uint8_t         data[2];
};

class MidiHandler
{
  public:
    MidiHandler() {}
    ~MidiHandler() {}

    // ## Functions

	// ### Init
    // Initializes the MidiHandler
	// ~~~~
    void Init();
	// ~~~~

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
    MidiEvent PopEvent()
    {
        return event_q_.Read();
    }

  private:
    enum ParserState
    {
        ParserEmpty,
        ParserHasStatus,
        ParserHasData0,
    };
    ParserState              pstate_;
    MidiEvent                incoming_message_;
    RingBuffer<MidiEvent, 8> event_q_;
};

} // namespace daisy

#endif
