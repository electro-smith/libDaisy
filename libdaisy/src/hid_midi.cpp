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
//     this could even bue as simple as a buffer/new flag.
void MidiHandler::Init(MidiInputMode in_mode, MidiOutputMode out_mode)
{
    in_mode_ = in_mode;
    out_mode_ = out_mode;
    uart_.Init();
    event_q_.Init();
    incoming_message_.type = MessageLast;
    pstate_                = MidiHandler::ParserEmpty;
}

void MidiHandler::StartReceive() 
{
	if(in_mode_ & INPUT_MODE_UART1) 
	{
		uart_.StartRx(6);
	}
}

void MidiHandler::Listen() {
    if(in_mode_ & INPUT_MODE_UART1)
    {
        while(uart_.Readable())
        {
            Parse(uart_.PopRx());
        }
    }
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

