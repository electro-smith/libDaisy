#pragma once
#ifndef DSY_MIDI_H
#define DSY_MIDI_H

// TODO: make this adjustable
#define SYSEX_BUFFER_LEN 1024

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
    SystemCommon,          /**< & */
    SystemRealTime,        /**< & */
    ChannelMode,           /**< & */
    MessageLast,           /**< & */
};

enum SystemCommonType
{
    SystemExclusive,     /**< & */
    MTCQuarterFrame,     /**< & */
    SongPositionPointer, /**< & */
    SongSelect,          /**< & */
    SCUndefined0,        /**< & */
    SCUndefined1,        /**< & */
    TuneRequest,         /**< & */
    SysExEnd,            /**< & */
    SystemCommonLast,    /**< & */
};

enum SystemRealTimeType
{
    TimingClock,        /**< & */
    SRTUndefined0,      /**< & */
    Start,              /**< & */
    Continue,           /**< & */
    Stop,               /**< & */
    SRTUndefined1,      /**< & */
    ActiveSensing,      /**< & */
    Reset,              /**< & */
    SystemRealTimeLast, /**< & */
};

enum ChannelModeType
{
    AllSoundOff,         /**< & */
    ResetAllControllers, /**< & */
    LocalControl,        /**< & */
    AllNotesOff,         /**< & */
    OmniModeOff,         /**< & */
    OmniModeOn,          /**< & */
    MonoModeOn,          /**< & */
    PolyModeOn,          /**< & */
    ChannelModeLast,     /**< & */
};

/** Struct containing note, and velocity data for a given channel.
Can be made from MidiEvent
*/
struct NoteOffEvent
{
    int     channel;  /**< & */
    uint8_t note;     /**< & */
    uint8_t velocity; /**< & */
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
/** Struct containing note, and pressure data for a given channel.
Can be made from MidiEvent
*/
struct PolyphonicKeyPressureEvent
{
    int     channel;
    uint8_t note;
    uint8_t pressure;
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
/** Struct containing new program number, for a given channel.
Can be made from MidiEvent
*/
struct ProgramChangeEvent
{
    int     channel; /**< & */
    uint8_t program; /**< & */
};
/** Struct containing pressure (aftertouch), for a given channel.
Can be made from MidiEvent
*/
struct ChannelPressureEvent
{
    int     channel;  /**< & */
    uint8_t pressure; /**< & */
};
/** Struct containing pitch bend value for a given channel.
Can be made from MidiEvent
*/
struct PitchBendEvent
{
    int     channel; /**< & */
    int16_t value;   /**< & */
};
/** Struct containing sysex data.
Can be made from MidiEvent
*/
struct SystemExclusiveEvent
{
    int     length;
    uint8_t data[SYSEX_BUFFER_LEN]; /**< & */
};
/** Struct containing QuarterFrame data.
Can be made from MidiEvent
*/
struct MTCQuarterFrameEvent
{
    uint8_t message_type; /**< & */
    uint8_t value;        /**< & */
};
/** Struct containing song position data.
Can be made from MidiEvent
*/
struct SongPositionPointerEvent
{
    uint16_t position; /**< & */
};
/** Struct containing song select data.
Can be made from MidiEvent
*/
struct SongSelectEvent
{
    uint8_t song; /**< & */
};
/** Struct containing sound off data.
Can be made from MidiEvent
*/
struct AllSoundOffEvent
{
    int channel; /**< & */
}
/** Struct containing ResetAllControllersEvent data.
Can be made from MidiEvent
*/
struct ResetAllControllersEvent
{
    int     channel; /**< & */
    uint8_t value;   /**< & */
}
/** Struct containing LocalControlEvent data.
Can be made from MidiEvent
*/
struct LocalControlEvent
{
    int  channel;           /**< & */
    bool local_control_off; /**< & */
    bool local_control_on;  /**< & */
}
/** Struct containing AllNotesOffEvent data.
Can be made from MidiEvent
*/
struct AllNotesOffEvent
{
    int channel; /**< & */
}
/** Struct containing OmniModeOffEvent data. 
 * Can be made from MidiEvent
*/
struct OmniModeOffEvent
{
    int channel; /**< & */
}
/** Struct containing OmniModeOnEvent data.
Can be made from MidiEvent
*/
struct OmniModeOnEvent
{
    int channel; /**< & */
}
/** Struct containing MonoModeOnEvent data.
Can be made from MidiEvent
*/
struct MonoModeOnEvent
{
    int     channel;      /**< & */
    uint8_t num_channels; /**< & */
}
/** Struct containing PolyModeOnEvent data.
Can be made from MidiEvent
*/
struct PolyModeOnEvent
{
    int channel; /**< & */
}


/** Simple MidiEvent with message type, channel, and data[2] members.
*/
struct MidiEvent
{
    // Newer ish.
    MidiMessageType    type;                         /**< & */
    int                channel;                      /**< & */
    uint8_t            data[2];                      /**< & */
    uint8_t            sysex_data[SYSEX_BUFFER_LEN]; /**< & */
    uint8_t            sysex_message_len;
    SystemCommonType   sc_type;
    SystemRealTimeType srt_type;
    ChannelModeType    cm_type;

    /** Returns the data within the MidiEvent as a NoteOffEvent struct */
    NoteOffEvent AsNoteOff()
    {
        NoteOffEvent m;
        m.channel  = channel;
        m.note     = data[0];
        m.velocity = data[1];
        return m;
    }

    /** Returns the data within the MidiEvent as a NoteOnEvent struct */
    NoteOnEvent AsNoteOn()
    {
        NoteOnEvent m;
        m.channel  = channel;
        m.note     = data[0];
        m.velocity = data[1];
        return m;
    }

    /** Returns the data within the MidiEvent as a PolyphonicKeyPressureEvent struct */
    PolyphonicKeyPressureEvent AsPolyphonicKeyPressure()
    {
        PolyphonicKeyPressureEvent m;
        m.channel  = channel;
        m.note     = data[0];
        m.pressure = data[1];
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

    /** Returns the data within the MidiEvent as a ProgramChangeEvent struct.*/
    ProgramChangeEvent AsProgramChange()
    {
        ProgramChangeEvent m;
        m.channel = channel;
        m.program = data[0];
        return m;
    }

    /** Returns the data within the MidiEvent as a ProgramChangeEvent struct.*/
    ChannelPressureEvent AsChannelPressure()
    {
        ChannelPressureEvent m;
        m.channel  = channel;
        m.pressure = data[0];
        return m;
    }

    /** Returns the data within the MidiEvent as a PitchBendEvent struct.*/
    PitchBendEvent AsPitchBend()
    {
        PitchBendEvent m;
        m.channel = channel;
        m.value   = ((uint16_t)data[1] << 7) + (data[0] - 8192);
        return m;
    }
    SystemExclusiveEvent AsSystemExclusive()
    {
        SystemExclusiveEvent m;
        m.length = sysex_message_len;
        for(int i = 0; i < SYSEX_BUFFER_LEN; i++)
        {
            m.data[i] = 0;
            if(i < m.length)
            {
                m.data[i] = sysex_data[i];
            }
        }
        return m;
    }
    MTCQuarterFrameEvent AsMTCQuarterFrame()
    {
        MTCQuarterFrameEvent m;
        m.message_type = (data[0] & 0x70) >> 4;
        m.value        = data[0] & 0x0f;
        return m;
    }
    SongPositionPointerEvent AsSongPositionPointer()
    {
        SongPositionPointerEvent m;
        m.position = ((uint16_t)data[1] << 7) + (data[0] - 8192);
        return m;
    }
    SongSelectEvent AsSongSelect()
    {
        SongSelectEvent m;
        m.song = data[0];
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
    void SendMessage(uint8_t* bytes, size_t size);

  private:
    enum ParserState
    {
        ParserEmpty,
        ParserHasStatus,
        ParserHasData0,
        ParserSysEx,
    };
    MidiInputMode              in_mode_;
    MidiOutputMode             out_mode_;
    UartHandler                uart_;
    ParserState                pstate_;
    MidiEvent                  incoming_message_;
    RingBuffer<MidiEvent, 256> event_q_;
    uint32_t                   last_read_; // time of last byte
    MidiMessageType            running_status_;

    void ClearSysExBuffer(MidiEvent* event);
};

/** @} */
} // namespace daisy
#endif
