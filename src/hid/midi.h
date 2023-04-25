#pragma once
#ifndef DSY_MIDI_H
#define DSY_MIDI_H

// TODO: make this adjustable
#define SYSEX_BUFFER_LEN 128

#include <stdint.h>
#include <stdlib.h>
#include "per/uart.h"
#include "util/ringbuffer.h"
#include "util/FIFO.h"
#include "hid/MidiEvent.h"
#include "hid/usb_midi.h"
#include "sys/system.h"

namespace daisy
{

/** @brief   Transport layer for sending and receiving MIDI data over UART
 *  @details This is the mode of communication used for TRS and DIN MIDI
 *           There is an additional 2kB of RAM data used within this class
 *           for processing bulk data from the UART peripheral
 *  @ingroup midi
*/
class MidiUartTransport
{
  public:

    typedef void(*MidiRxParseCallback)(uint8_t *data, size_t size, void *context);

    MidiUartTransport() {}
    ~MidiUartTransport() {}

    /** @brief Configuration structure for UART MIDI */
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

    /** @brief Initialization of UART using config struct */
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

    /** @brief Start the UART peripheral in listening mode. This will fill an internal data structure in the background */
    inline void StartRx(MidiRxParseCallback parse_callback, void *context)
    {
        parse_context_ = context;
        parse_callback_ = parse_callback;
        uart_.DmaListenStart(
            rx_buffer, kDataSize, MidiUartTransport::rxCallback, this);
    }

    /** @brief returns whether the UART peripheral is actively listening in the background or not */
    inline bool RxActive() { return uart_.IsListening(); }

    /** @brief This is a no-op for UART transport - Rx is via DMA callback with circular buffer */
    inline void FlushRx() {}

    /** @brief sends the buffer of bytes out of the UART peripheral */
    inline void Tx(uint8_t* buff, size_t size) { uart_.PollTx(buff, size); }

    /** This size determines the maximum Rx bytes readable by the UART in the background
     *  These will fill a software FIFO that can be parsed. The FIFO is twice the size of the
     *  Rx buffer.
     */
    private:
        static constexpr size_t      kDataSize = 256;
        UartHandler                  uart_;
        uint8_t                      rx_buffer[kDataSize];
        void*                        parse_context_;
        MidiRxParseCallback          parse_callback_;

        /** Static callback for Uart MIDI that occurs when
         *  new data is available from the peripheral.
         *  The new data is transferred from the peripheral to the
         *  MIDI instance's byte FIFO that feeds the MIDI parser.
         *
         *  TODO: Handle UartHandler errors better/at all.
         *  (If there is a UART error, there's not really any recovery
         *  option at the moment)
         */
        static void rxCallback(uint8_t*            data,
                               size_t              size,
                               void*               context,
                               UartHandler::Result res)
        {
            /** Read context as transport type */
            MidiUartTransport* transport
                = reinterpret_cast<MidiUartTransport*>(context);
            if(res == UartHandler::Result::OK)
            {
                if (transport->parse_callback_)
                {
                    transport->parse_callback_(data, size, transport->parse_context_);
                }
            }
        }
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

        //event_q_.Init();
        incoming_message_.type = MessageLast;
        pstate_                = ParserEmpty;
    }

    /** Starts listening on the selected input mode(s). MidiEvent Queue will begin to fill, and can be checked with HasEvents() */
    void StartReceive()
    {
        transport_.StartRx(MidiHandler::ParseCallback, this);
    }

    /** Start listening */
    void Listen()
    {
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
    //bool HasEvents() const { return event_q_.readable(); }
    bool HasEvents() const { return event_q_.GetNumElements() > 0; }


    /** Pops the oldest unhandled MidiEvent from the internal queue
    \return The event to be handled
     */
    // MidiEvent PopEvent() { return event_q_.Read(); }
    MidiEvent PopEvent() { return event_q_.PopFront(); }

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
        // reset parser when status byte is received
        if((byte & kStatusByteMask) && pstate_ != ParserSysEx)
        {
            pstate_ = ParserEmpty;
        }
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
                                event_q_.PushBack(incoming_message_);
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
                                    event_q_.PushBack(incoming_message_);
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
                    //check for single byte running status, really this only applies to channel pressure though
                    if(running_status_ == ChannelPressure
                       || running_status_ == ProgramChange
                       || incoming_message_.sc_type == MTCQuarterFrame
                       || incoming_message_.sc_type == SongSelect)
                    {
                        //Send the single byte update
                        pstate_ = ParserEmpty;
                        event_q_.PushBack(incoming_message_);
                    }
                    else
                    {
                        pstate_
                            = ParserHasData0; //we need to get the 2nd byte yet.
                    }
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
                        event_q_.PushBack(incoming_message_);
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
                        incoming_message_.type = NoteOff;
                    }

                    // At this point the message is valid, and we can add this MidiEvent to the queue
                    event_q_.PushBack(incoming_message_);
                }
                else
                {
                    // invalid message go back to start ;p
                    pstate_ = ParserEmpty;
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
                    event_q_.PushBack(incoming_message_);
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
    ParserState          pstate_;
    MidiEvent            incoming_message_;
    FIFO<MidiEvent, 256> event_q_;
    uint32_t             last_read_; // time of last byte
    MidiMessageType      running_status_;
    Config               config_;
    Transport            transport_;

    // Masks to check for message type, and byte content
    const uint8_t kStatusByteMask     = 0x80;
    const uint8_t kMessageMask        = 0x70;
    const uint8_t kDataByteMask       = 0x7F;
    const uint8_t kSystemCommonMask   = 0xF0;
    const uint8_t kChannelMask        = 0x0F;
    const uint8_t kRealTimeMask       = 0xF8;
    const uint8_t kSystemRealTimeMask = 0x07;

    static void ParseCallback(uint8_t *data, size_t size, void *context)
    {
        MidiHandler *handler = reinterpret_cast<MidiHandler*>(context);
        for (size_t i = 0; i < size; i++)
        {
            handler->Parse(data[i]);
        }
    }
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
