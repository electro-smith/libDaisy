#pragma once
#ifndef DSY_MIDI_H
#define DSY_MIDI_H

#include <stdint.h>
#include <stdlib.h>

namespace daisy 
{
struct MidiEvent
{
    bool is_note() { return note_ > -1 ? true : false; }
    bool is_cc() { return cc_ > -1 ? true : false; }
    int note_, vel_, cc_, val_;
}; 
class MidiHandler
{
  public:
    // Channel Specific Messages
    enum Message
    {
        NoteOff,
        NoteOn,
        PolyphonicKeyPressure,
        ControlChange,
        ProgramChange,
        ChannelPressure,
        PitchBend,
        MessageLast,
    };

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
