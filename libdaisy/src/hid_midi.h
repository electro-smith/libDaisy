#pragma once
#ifndef DSY_MIDI_H
#define DSY_MIDI_H

#include <stdint.h>
#include <stdlib.h>

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

    // Parses an Incoming Message
    MidiEvent Parse(uint8_t *buffer, size_t size);

  private:
};

} // namespace daisy

#endif
