#pragma once
#ifndef DSY_MIDI_H
#define DSY_MIDI_H

#include <stdint.h>
#include <stdlib.h>

namespace daisy 
{


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
    void Parse(uint8_t *buffer, size_t size);

  private:



}

} // namespace daisy

#endif
