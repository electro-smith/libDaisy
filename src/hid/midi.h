#pragma once
#ifndef DSY_MIDI_H
#define DSY_MIDI_H

// TODO: make this adjustable
#define SYSEX_BUFFER_LEN 128

#include <stdint.h>
#include <stdlib.h>
#include "per/uart.h"
#include "util/ringbuffer.h"
#include "hid/MidiEvent.h"
#include "hid/usb_midi.h"
#include "sys/system.h"

namespace daisy
{
/** @brief   Transport layer for sending and receiving MIDI data over UART 
 *  @details This is the mode of communication used for TRS and DIN MIDI
 *  @ingroup midi
*/
class MidiUartTransport
{
  public:
    MidiUartTransport() {}
    ~MidiUartTransport() {}

    struct Config
    {
        UartHandler::Config::Peripheral periph;
        dsy_gpio_pin                    rx;
        dsy_gpio_pin                    tx;

        Config()
        {
            periph = UartHandler::Config::Peripheral::USART_1;
            rx     = {DSY_GPIOB, 7};
            tx     = {DSY_GPIOB, 6};
        }
    };

    inline void Init(Config config)
    {
        UartHandler::Config uart_config;

        //defaults
        uart_config.baudrate   = 31250;
        uart_config.stopbits   = UartHandler::Config::StopBits::BITS_1;
        uart_config.parity     = UartHandler::Config::Parity::NONE;
        uart_config.mode       = UartHandler::Config::Mode::TX_RX;
        uart_config.wordlength = UartHandler::Config::WordLength::BITS_8;

        //user settings
        uart_config.periph        = config.periph;
        uart_config.pin_config.rx = config.rx;
        uart_config.pin_config.tx = config.tx;

        uart_.Init(uart_config);
    }

    inline void    StartRx() { uart_.StartRx(); }
    inline size_t  Readable() { return uart_.Readable(); }
    inline uint8_t Rx() { return uart_.PopRx(); }
    inline bool    RxActive() { return uart_.RxActive(); }
    inline void    FlushRx() { uart_.FlushRx(); }
    inline void    Tx(uint8_t* buff, size_t size) { uart_.PollTx(buff, size); }

  private:
    UartHandler uart_;
};

/** 
    @brief Simple MIDI Handler \n 
    Parses bytes from an input into valid MidiEvents. \n 
    The MidiEvents fill a FIFO queue that the user can pop messages from.
    @author shensley
    @date March 2020
    @ingroup midi
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
     *  \param config Configuration structure used to define specifics to the MIDI Handler.
     */
    void Init(Config config)
    {
        config_ = config;

        transport_.Init(config_.transport_config);

        event_q_.Init();
        incoming_message_.type = MessageLast;
        pstate_                = ParserEmpty;
    }

    /** Starts listening on the selected input mode(s). MidiEvent Queue will begin to fill, and can be checked with HasEvents() */
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
                            if(byte & 0x08)
                            {
                                incoming_message_.type = SystemRealTime;
                                running_status_        = SystemRealTime;
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
                        running_status_        = ChannelMode;
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

                    //velocity 0 NoteOns are NoteOffs
                    if(running_status_ == NoteOn
                       && incoming_message_.data[1] == 0)
                    {
                        incoming_message_.type = running_status_ = NoteOff;
                    }

                    // At this point the message is valid, and we can add this MidiEvent to the queue
                    event_q_.Write(incoming_message_);
                }
                // Regardless, of whether the data was valid or not we go back to empty
                // because either the message is queued for handling or its not.
                pstate_ = ParserEmpty;
                break;
            case ParserSysEx:
                // end of sysex
                if(byte == 0xf7)
                {
                    pstate_ = ParserEmpty;
                    event_q_.Write(incoming_message_);
                }
                else if(incoming_message_.sysex_message_len < SYSEX_BUFFER_LEN)
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

/**
 *  @{ 
 *  @ingroup midi
 *  @brief shorthand accessors for MIDI Handlers
 * */
using MidiUartHandler = MidiHandler<MidiUartTransport>;
using MidiUsbHandler  = MidiHandler<MidiUsbTransport>;
/** @} */
} // namespace daisy
#endif
