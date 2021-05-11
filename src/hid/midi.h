#pragma once
#ifndef DSY_MIDI_H
#define DSY_MIDI_H

#include <stdint.h>
#include <stdlib.h>
#include "per/uart.h"
#include "util/ringbuffer.h"

namespace daisy
{
/** @addtogroup external 
    @{ 
*/


/** Parsed from the Status Byte, these are the common Midi Messages that can be handled. \n
At this time only 3-byte messages are correctly parsed into MidiEvents.
*/
enum MidiMessageType
{
    NoteOff,               /**< & */
    NoteOn,                /**< & */
    PolyphonicKeyPressure, /**< & */
    ControlChange,         /**< & */
    ProgramChange,         /**< & */
    ChannelPressure,       /**< & */
    PitchBend,             /**< & */
    MessageLast,
    /**< & */ // maybe change name to MessageUnsupported
};

/** Struct containing note, and velocity data for a given channel.
Can be made from MidiEvent
*/
struct NoteOnEvent
{
    int     channel;  /**< & */
    uint8_t note;     /**< & */
    uint8_t velocity; /**< & */
};
/** Struct containing control number, and value for a given channel.
Can be made from MidiEvent
*/
struct ControlChangeEvent
{
    int     channel;        /**< & */
    uint8_t control_number; /**< & */
    uint8_t value;          /**< & */
};
/** Struct containing pitch bend value for a given channel.
Can be made from MidiEvent
*/
struct PitchBendEvent
{
    int     channel; /**< & */
    int16_t value;   /**< & */
};

/** Simple MidiEvent with message type, channel, and data[2] members.
*/
struct MidiEvent
{
    // Newer ish.
    MidiMessageType type;    /**< & */
    int             channel; /**< & */
    uint8_t         data[2]; /**< & */

    /** Returns the data within the MidiEvent as a NoteOnEvent struct */
    NoteOnEvent AsNoteOn()
    {
        NoteOnEvent m;
        m.channel  = channel;
        m.note     = data[0];
        m.velocity = data[1];
        return m;
    }

    /** Returns the data within the MidiEvent as a ControlChangeEvent struct.*/
    ControlChangeEvent AsControlChange()
    {
        ControlChangeEvent m;
        m.channel        = channel;
        m.control_number = data[0];
        m.value          = data[1];
        return m;
    }

    /** Returns the data within the MidiEvent as a PitchBendEvent struct.*/
    PitchBendEvent AsPitchBend()
    {
        PitchBendEvent m;
        m.channel = channel;
        m.value   = ((uint16_t)data[1] << 7) + data[0] - 8192;
        return m;
    }
};

/** 
    @brief Simple MIDI Handler \n 
    Parses bytes from an input into valid MidiEvents. \n 
    The MidiEvents fill a FIFO queue that the user can pop messages from.
    @author shensley
    @date March 2020
*/
class MidiHandler
{
  public:
    MidiHandler() {}
    ~MidiHandler() {}
    /** Input and Output can be configured separately
    Multiple Input modes can be selected by OR'ing the values.
    */
    enum MidiInputMode
    {
        INPUT_MODE_NONE    = 0x00, /**< & */
        INPUT_MODE_UART1   = 0x01, /**< & */
        INPUT_MODE_USB_INT = 0x02, /**< & */
        INPUT_MODE_USB_EXT = 0x04, /**< & */
    };
    /** Output mode */
    enum MidiOutputMode
    {
        OUTPUT_MODE_NONE    = 0x00, /**< & */
        OUTPUT_MODE_UART1   = 0x01, /**< & */
        OUTPUT_MODE_USB_INT = 0x02, /**< & */
        OUTPUT_MODE_USB_EXT = 0x04, /**< & */
    };


    /** Initializes the MidiHandler 
    \param in_mode Input mode
    \param out_mode Output mode
     */
    void Init(MidiInputMode in_mode, MidiOutputMode out_mode);

    /** Starts listening on the selected input mode(s). MidiEvent Queue will begin to fill, and can be checked with */
    void StartReceive();

    /** Start listening */
    void Listen();

    /** Feed in bytes to state machine from a queue.
    Populates internal FIFO queue with MIDI Messages
    For example with uart:
    midi.Parse(uart.PopRx());
    \param byte &
    */
    void Parse(uint8_t byte);

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
    void SendMessage(uint8_t *bytes, size_t size);


  private:
    enum ParserState
    {
        ParserEmpty,
        ParserHasStatus,
        ParserHasData0,
    };
    MidiInputMode              in_mode_;
    MidiOutputMode             out_mode_;
    UartHandler                uart_;
    ParserState                pstate_;
    MidiEvent                  incoming_message_;
    RingBuffer<MidiEvent, 256> event_q_;
    uint32_t                   last_read_; // time of last byte
    MidiMessageType            running_status_;
};

/** @} */
} // namespace daisy
#endif
