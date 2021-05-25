#include "hid/midi.h"
#include "sys/system.h"

using namespace daisy;

// Masks to check for message type, and byte content
const uint8_t kStatusByteMask     = 0x80;
const uint8_t kMessageMask        = 0x70;
const uint8_t kDataByteMask       = 0x7F;
const uint8_t kSystemCommonMask   = 0xF0;
const uint8_t kChannelMask        = 0x0F;
const uint8_t kRealTimeMask       = 0xF8;
const uint8_t kSystemRealTimeMask = 0x07;

// Currently only setting this up to handle 3-byte messages (i.e. Notes, CCs, Pitchbend).
// We'll have to do some minor tweaking to handle program changes, sysex,
//    realtime messages, and most system common messages.

// TODO:
// - provide an input interface so USB or UART data can be passed in.
//     this could even bue as simple as a buffer/new flag.
void MidiHandler::Init(MidiInputMode in_mode, MidiOutputMode out_mode)
{
    in_mode_  = in_mode;
    out_mode_ = out_mode;

    UartHandler::Config config;
    config.baudrate      = 31250;
    config.periph        = UartHandler::Config::Peripheral::USART_1;
    config.stopbits      = UartHandler::Config::StopBits::BITS_1;
    config.parity        = UartHandler::Config::Parity::NONE;
    config.mode          = UartHandler::Config::Mode::TX_RX;
    config.wordlength    = UartHandler::Config::WordLength::BITS_8;
    config.pin_config.rx = {DSY_GPIOB, 7};
    config.pin_config.tx = {DSY_GPIOB, 6};
    uart_.Init(config);

    event_q_.Init();
    incoming_message_.type = MessageLast;
    pstate_                = ParserEmpty;
}

void MidiHandler::StartReceive()
{
    if(in_mode_ & INPUT_MODE_UART1)
    {
        uart_.StartRx();
    }
}

void MidiHandler::Listen()
{
    uint32_t now;
    now = System::GetNow();
    if(in_mode_ & INPUT_MODE_UART1)
    {
        while(uart_.Readable())
        {
            last_read_ = now;
            Parse(uart_.PopRx());
        }

        // In case of UART Error, (particularly
        //  overrun error), UART disables itself.
        // Flush the buff, and restart.
        if(!uart_.RxActive())
        {
            pstate_ = ParserEmpty;
            uart_.FlushRx();
            StartReceive();
        }
    }
}


void MidiHandler::Parse(uint8_t byte)
{
    switch(pstate_)
    {
        case ParserEmpty:
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
                    pstate_ = ParserHasStatus;
                    // Mark this status byte as running_status
                    running_status_ = incoming_message_.type;

                    if(running_status_ == SystemCommon)
                    {
                        incoming_message_.channel = 0;
                        //system real time = 1111 1xxx
                        if((byte & 0x08) > 0)
                        {
                            incoming_message_.type = SystemRealTime;
                            incoming_message_.srt_type
                                = static_cast<SystemRealTimeType>(
                                    byte & kSystemRealTimeMask);

                            //short circuit to start
                            event_q_.Write(incoming_message_);
                            pstate_ = ParserEmpty;
                        }
                        //system common
                        else
                        {
                            incoming_message_.sc_type
                                = static_cast<SystemCommonType>(byte & 0x07);
                            //sysex
                            if(incoming_message_.sc_type == SystemExclusive)
                            {
                                pstate_ = ParserSysEx;
                                incoming_message_.sysex_message_len = 0;
                            }
                            //short circuit
                            else if(incoming_message_.sc_type > SongSelect)
                            {
                                event_q_.Write(incoming_message_);
                                pstate_ = ParserEmpty;
                            }
                        }
                    }
                }
                // Else we'll keep waiting for a valid incoming status byte
            }
            else
            {
                // Handle as running status
                incoming_message_.type    = running_status_;
                incoming_message_.data[0] = byte & kDataByteMask;
                pstate_                   = ParserHasData0;
            }
            break;
        case ParserHasStatus:
            if((byte & kStatusByteMask) == 0)
            {
                incoming_message_.data[0] = byte & kDataByteMask;
                if(incoming_message_.type == ChannelPressure
                   || incoming_message_.type == ProgramChange
                   || incoming_message_.sc_type == MTCQuarterFrame
                   || incoming_message_.sc_type == SongSelect)
                {
                    //these are just one data byte, so we short circuit back to start
                    pstate_ = ParserEmpty;
                    event_q_.Write(incoming_message_);
                }
                else
                {
                    pstate_ = ParserHasData0;
                }

                //ChannelModeMessages (reserved Control Changes)
                if(incoming_message_.type == ControlChange
                   && incoming_message_.data[0] > 119)
                {
                    incoming_message_.type    = ChannelMode;
                    incoming_message_.cm_type = static_cast<ChannelModeType>(
                        incoming_message_.data[0] - 120);
                }
            }
            else
            {
                // invalid message go back to start ;p
                pstate_ = ParserEmpty;
            }
            break;
        case ParserHasData0:
            if((byte & kStatusByteMask) == 0)
            {
                incoming_message_.data[1] = byte & kDataByteMask;
                // At this point the message is valid, and we can add this MidiEvent to the queue
                event_q_.Write(incoming_message_);
            }
            // Regardless, of whether the data was valid or not we go back to empty
            // because either the message is queued for handling or its not.
            pstate_ = ParserEmpty;
            break;
        case ParserSysEx:
            // end of sysex
            if(byte == 0xf7
               || incoming_message_.sysex_message_len >= SYSEX_BUFFER_LEN)
            {
                pstate_ = ParserEmpty;
                event_q_.Write(incoming_message_);
            }
            else
            {
                incoming_message_
                    .sysex_data[incoming_message_.sysex_message_len]
                    = byte;
                incoming_message_.sysex_message_len++;
            }
            break;
        default: break;
    }
}

void MidiHandler::SendMessage(uint8_t *bytes, size_t size)
{
    if(out_mode_ == OUTPUT_MODE_UART1)
    {
        uart_.PollTx(bytes, size);
    }
}