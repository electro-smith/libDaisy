#include "hid_midi.h"

using namespace daisy;

// Masks to check for message type, and byte content
const uint8_t kStatusByteMask   = 0x80;
const uint8_t kMessageMask      = 0x70;
const uint8_t kDataByteMask     = 0x7F;
const uint8_t kSystemCommonMask = 0xF0;
const uint8_t kChannelMask      = 0x0F;
const uint8_t kRealTimeMask     = 0xF8;

// Currently only setting this up to handle 3-byte messages (i.e. Notes, CCs, Pitchbend).
// We'll have to do some minor tweaking to handle program changes, sysex,
//	realtime messages, and most system common messages.

// TODO:
// - provide an input interface so USB or UART data can be passed in.
//     this could even be as simple as a buffer/new flag.
void MidiHandler::Init()
{
    event_q_.Init();
    incoming_message_.type = MessageLast;
    pstate_                = MidiHandler::ParserEmpty;
}

void MidiHandler::Parse(uint8_t byte)
{
    switch(pstate_)
    {
        case MidiHandler::ParserEmpty:
            // check byte for valid Status Byte
            if(byte & kStatusByteMask)
            {
                // Get MessageType, and Channel
                incoming_message_.channel = byte & kChannelMask;
                incoming_message_.type
                    = static_cast<MidiMessageType>((byte & kMessageMask) >> 4);
                // Validate, and move on.
                if(incoming_message_.type < MessageLast)
                {
                    pstate_ = MidiHandler::ParserHasStatus;
                }
                // Else we'll keep waiting for a valid incoming status byte
            }
            break;
        case MidiHandler::ParserHasStatus:
            if((byte & kStatusByteMask) == 0)
            {
                incoming_message_.data[0] = byte & kDataByteMask;
                pstate_                   = MidiHandler::ParserHasData0;
            }
            else
            {
                // invalid message go back to start ;p
                pstate_ = MidiHandler::ParserEmpty;
            }
            break;
        case MidiHandler::ParserHasData0:
            if((byte & kStatusByteMask) == 0)
            {
                incoming_message_.data[1] = byte & kDataByteMask;
                // At this point the message is valid, and we can add this MidiEvent to the queue
                event_q_.Write(incoming_message_);
            }
            // Regardless, of whether the data was valid or not we go back to empty
            // because either the message is queued for handling or its not.
            pstate_ = MidiHandler::ParserEmpty;
            break;
        default: break;
    }
}

// TODO:
// double check for packet size...
// right now I'm only handling one message
// I have a feeling a packet can contain many
MidiEvent MidiHandler::ParseMessage(uint8_t *buffer, size_t size)
{
    // check first byte
    uint8_t sb, *db, channel;
    size_t  expected_packet_size;
    bool    valid;
    // Get bytes in place

    sb      = (buffer[0] & kMessageMask) >> 4;
    channel = (buffer[0] & kChannelMask);
    db      = buffer + 1;
    // Check Validity
    // TODO: Check number of size for valid MIDI Message
    valid = sb < MessageLast ? true : false;
    // Vars for data.
    // TODO: Move this to a MidiEvent class of some sort that can be passed
    //         out of the parser.
    uint8_t   note, velocity; // For note messages
    uint8_t   cc, val;
    int16_t   pitch_bend;
    MidiEvent event;
    event.note_ = -1;
    event.vel_  = -1;
    event.cc_   = -1;
    event.val_  = -1;
    switch(sb)
    {
        case NoteOn:
        case NoteOff:
        case PolyphonicKeyPressure:
            if(size <= 3)
            {
                event.note_ = db[0];
                event.vel_  = db[1];
            }
            break;
        case ControlChange:
            if(size <= 3)
            {
                event.cc_  = db[0];
                event.val_ = db[1];
            }
            break;
        case ProgramChange: break;
        case ChannelPressure: break;
        case PitchBend: break;
        default: break;
    }
    // TODO add output..
    return event;
}
