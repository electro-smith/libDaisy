#pragma once
#ifndef DSY_MIDI_H
#define DSY_MIDI_H

// TODO: make this adjustable
#define SYSEX_BUFFER_LEN 100

#include <stdint.h>
#include <stdlib.h>
#include "per/uart.h"
#include "util/ringbuffer.h"
#include "hid/MidiEvent.h"

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

class MidiUartTransport
{
  public:
    MidiUartTransport() {}
    ~MidiUartTransport() {}

    inline void    Init(UartHandler::Config config) { uart_.Init(config); }
    inline void    StartRx() { uart_.StartRx(); }
    inline size_t  Readable() { return uart_.Readable(); }
    inline uint8_t Rx() { return uart_.PopRx(); }
    inline bool    RxActive() { return uart_.RxActive(); }
    inline void    FlushRx() { uart_.FlushRx(); }
    inline void    Tx(uint8_t* buff, size_t size) { uart_.PollTx(buff, size); }

    struct Config
    {
        UartHandler::Config periph_config;

        void Default()
        {
            periph_config.baudrate   = 31250;
            periph_config.periph     = UartHandler::Config::Peripheral::USART_1;
            periph_config.stopbits   = UartHandler::Config::StopBits::BITS_1;
            periph_config.parity     = UartHandler::Config::Parity::NONE;
            periph_config.mode       = UartHandler::Config::Mode::TX_RX;
            periph_config.wordlength = UartHandler::Config::WordLength::BITS_8;
            periph_config.pin_config.rx = {DSY_GPIOB, 7};
            periph_config.pin_config.tx = {DSY_GPIOB, 6};
        }
    };

  private:
    UartHandler uart_;
};

/** Fake transport for testing the MidiHandler */
class MidiTestTransport{
    public:
    MidiTestTransport() {}
    ~MidiTestTransport() {}

    //stubs just to trick the MidiHandler
    inline void    StartRx() {}
    inline size_t  Readable() { return msg_idx_ < 149; }
    inline bool    RxActive() { return true; }
    inline void    FlushRx() {}
    inline void    Tx(uint8_t* buff, size_t size) {}

    inline void    Init(int fake_config) { msg_idx_ = 0; }

    inline uint8_t Rx() {
        uint8_t ret = messages_[msg_idx_];
        msg_idx_++;
        return ret;
    }


    struct Config
    {
        int periph_config; //does nothing
    };

  private:
    int msg_idx_;

    //for now all on channel 0
    uint8_t messages_[149] = 
    {
        // ============ Channel Voice Messages ==============

        0x80, 0x00, 0x00, // Note Off, key 0, vel 0
        0x80, 0x40, 0x40, // Note Off, key 64, vel 64
        0x80, 0x7f, 0x7f, // Note Off, key 127, vel 127

        0x90, 0x00, 0x00, // Note On, key 0, vel 0
        0x90, 0x40, 0x40, // Note On, key 64, vel 64
        0x90, 0x7f, 0x7f, // Note On, key 127, vel 127

        0xa0, 0x00, 0x00, // PKP, key 0, vel 0
        0xa0, 0x40, 0x40, // PKP, key 64, vel 64
        0xa0, 0x7f, 0x7f, // PKP, key 127, vel 127

        0xb0, 0x00, 0x00, // Control Change, cc 0, val 0
        0xb0, 0x40, 0x40, // Control Change, cc 64, val 64
        0xb0, 0x77, 0x7f, // Control Change, cc 119, val 127

        0xc0, 0x00,  // Program Change, program 0
        0xc0, 0x40,  // Program Change, program 64
        0xc0, 0x7f,  // Program Change, program 127

        0xd0, 0x00,  // Channel Pressure, program 0
        0xd0, 0x40,  // Channel Pressure, program 64
        0xd0, 0x7f,  // Channel Pressure, program 127

        0xe0, 0x00, 0x00, // Pitch Bend, val -8192
        0xe0, 0x00, 0x40, // Pitch Bend, val 0
        0xe0, 0x7f, 0x7f, // Pitch Bend, val 8191

        // ============ Channel Mode Messages ==============

        0xb0, 0x78, 0x00, // All Sound Off, value 0

        0xb0, 0x79, 0x00, // Reset All Controllers, value 0
        0xb0, 0x79, 0x40, // Reset All Controllers, value 64
        0xb0, 0x79, 0x7f, // Reset All Controllers, value 127

        0xb0, 0x7a, 0x00, // Local Control, value Off
        0xb0, 0x7a, 0x7f, // Local Control, value On

        0xb0, 0x7b, 0x00, // All Notes Off, value 0

        0xb0, 0x7c, 0x00, // Omni Mode Off
        0xb0, 0x7d, 0x00, // Omni Mode On

        0xb0, 0x7e, 0x00, // Mono Mode On, value 0
        0xb0, 0x7e, 0x40, // Mono Mode On, value 64
        0xb0, 0x7e, 0x7f, // Mono Mode On, value 127

        0xb0, 0x7f, 0x00, // Poly Mode On, value 0
    
        // ============ System Common Messages (minus sysex) ==============
        
        0xf1, 0x00, // MTC Quarter Frame, type 0, value 0
        0xf1, 0x33, // MTC Quarter Frame, type 3, value 3
        0xf1, 0x7f, // MTC Quarter Frame, type 7, value 15

        0xf2, 0x00, 0x00, // Song Position Pointer, value 0
        0xf2, 0x40, 0x40, // Song Position Pointer, value 8256
        0xf2, 0x77, 0x77, // Song Position Pointer, value 16383

        0xf3, 0x00, // Song Select, value 0
        0xf3, 0x40, // Song Select, value 64
        0xf3, 0x7f, // Song Select, value 127
    
        0xf4,  // Undefined
        0xf5,  // Undefined
        0xf6,  // Tune Request
        0xf7,  // End of Exclusive

        // ============ System Real Time ==============
        0xf8,  // Timing Clock
        0xf9,  // Undefined
        0xfa,  // Start
        0xfb,  // Continue
        0xfc,  // Stop
        0xfd,  // Undefined
        0xfe,  // Active Sensing
        0xff,  // Reset
    
        // ============ System Exclusive ==============
        0xf0, 0xf7, // empty
        0xf0, 0x00, 0x01, 0x02, 0xf7, // 0, 1, 2
        0xf0, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0xf7, // 123, 124, 125, 126, 127
    };
};


/** 
    @brief Simple MIDI Handler \n 
    Parses bytes from an input into valid MidiEvents. \n 
    The MidiEvents fill a FIFO queue that the user can pop messages from.
    @author shensley
    @date March 2020
*/
template <typename Transport>
class MidiHandler
{
  public:
    MidiHandler() {}
    ~MidiHandler() {}

    struct Config
    {
        typename Transport::Config transport_config;
    };

    /** Initializes the MidiHandler 
    \param in_mode Input mode
    \param out_mode Output mode
     */
    void Init(Config config)
    {
        config_ = config;

        transport_.Init(config_.transport_config.periph_config);

        event_q_.Init();
        incoming_message_.type = MessageLast;
        pstate_                = ParserEmpty;
    }

    /** Starts listening on the selected input mode(s). MidiEvent Queue will begin to fill, and can be checked with */
    void StartReceive() { transport_.StartRx(); }

    /** Start listening */
    void Listen()
    {
        uint32_t now;
        now = System::GetNow();
        while(transport_.Readable())
        {
            last_read_ = now;
            Parse(transport_.Rx());
        }

        // In case of UART Error, (particularly
        //  overrun error), UART disables itself.
        // Flush the buff, and restart.
        if(!transport_.RxActive())
        {
            pstate_ = ParserEmpty;
            transport_.FlushRx();
            StartReceive();
        }
    }

    /** Checks if there are unhandled messages in the queue 
    \return True if there are events to be handled, else false.
     */
    bool HasEvents() const { return event_q_.readable(); }


    /** Pops the oldest unhandled MidiEvent from the internal queue
    \return The event to be handled
     */
    MidiEvent PopEvent() { return event_q_.Read(); }

    /** SendMessage
    Send raw bytes as message
    */
    void SendMessage(uint8_t* bytes, size_t size)
    {
        transport_.Tx(bytes, size);
    }

    /** Feed in bytes to state machine from a queue.
    Populates internal FIFO queue with MIDI Messages
    For example with uart:
    midi.Parse(uart.PopRx());
    \param byte &
    */
    void Parse(uint8_t byte)
    {
        switch(pstate_)
        {
            case ParserEmpty:
                // check byte for valid Status Byte
                if(byte & kStatusByteMask)
                {
                    // Get MessageType, and Channel
                    incoming_message_.channel = byte & kChannelMask;
                    incoming_message_.type    = static_cast<MidiMessageType>(
                        (byte & kMessageMask) >> 4);

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
                                running_status_ = SystemRealTime;
                                incoming_message_.srt_type
                                    = static_cast<SystemRealTimeType>(
                                        byte & kSystemRealTimeMask);

                                //short circuit to start
                                pstate_ = ParserEmpty;
                                event_q_.Write(incoming_message_);
                            }
                            //system common
                            else
                            {
                                incoming_message_.sc_type
                                    = static_cast<SystemCommonType>(byte
                                                                    & 0x07);
                                //sysex
                                if(incoming_message_.sc_type == SystemExclusive)
                                {
                                    pstate_ = ParserSysEx;
                                    incoming_message_.sysex_message_len = 0;
                                }
                                //short circuit
                                else if(incoming_message_.sc_type > SongSelect)
                                {
                                    pstate_ = ParserEmpty;
                                    event_q_.Write(incoming_message_);
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
                    if(running_status_ == ChannelPressure
                       || running_status_ == ProgramChange
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
                    if(running_status_ == ControlChange
                       && incoming_message_.data[0] > 119)
                    {
                        incoming_message_.type = ChannelMode;
                        running_status_ = ChannelMode;
                        incoming_message_.cm_type
                            = static_cast<ChannelModeType>(
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

  private:
    enum ParserState
    {
        ParserEmpty,
        ParserHasStatus,
        ParserHasData0,
        ParserSysEx,
    };
    UartHandler                uart_;
    ParserState                pstate_;
    MidiEvent                  incoming_message_;
    RingBuffer<MidiEvent, 256> event_q_;
    uint32_t                   last_read_; // time of last byte
    MidiMessageType            running_status_;
    Config                     config_;
    Transport                  transport_;

    // Masks to check for message type, and byte content
    const uint8_t kStatusByteMask     = 0x80;
    const uint8_t kMessageMask        = 0x70;
    const uint8_t kDataByteMask       = 0x7F;
    const uint8_t kSystemCommonMask   = 0xF0;
    const uint8_t kChannelMask        = 0x0F;
    const uint8_t kRealTimeMask       = 0xF8;
    const uint8_t kSystemRealTimeMask = 0x07;
};

/** @} */

using MidiUartHandler = MidiHandler<MidiUartTransport>;
} // namespace daisy
#endif
