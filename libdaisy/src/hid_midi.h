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

struct MidiEvent
{
	// Currently working primitive
    bool is_note() { return note_ > -1 ? true : false; }
    bool is_cc() { return cc_ > -1 ? true : false; }
    int note_, vel_, cc_, val_;
	// Newer ish.
    MidiMessageType type;
    int             channel;
    uint8_t         data[2];
};

class MidiHandler
{
  public:
    // Channel Specific Messages

    MidiHandler() {}
    ~MidiHandler() {}

    // Initializes the MidiHandler
    void Init();

    // Feed in bytes to state machine.
    void Parse(uint8_t byte);

    // Parses an Incoming Message
	// To be deprectated I think.
	// or at least changed to just a loop over Parse()
	// -- too much duplication and slight difference.
    MidiEvent ParseMessage(uint8_t *buffer, size_t size);

    bool HasEvents() const { return event_q_.readable(); }

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
