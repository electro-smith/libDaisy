#pragma once

// This now defines the maximum cumulative length of buffered sysex
// data per midi parser, in bytes. It can be increased if the application
// is unable to consume sysex bytes fast enough to keep the buffer from overflowing.
#define SYSEX_BUF_MAX_SIZE 1024

// This is the max chunk length of sysex data enqueued in each parsed event.
// The event may not represent all the sysex data in a given transfer.
// Application must handle streamed parsing of multiple chunks.
#define SYSEX_BUF_CHUNK_LEN 128

static_assert(SYSEX_BUF_MAX_SIZE % SYSEX_BUF_CHUNK_LEN == 0);

#include <stdint.h>
#include "util/ringbuffer.h"

namespace daisy
{
/** @addtogroup midi MIDI
 *  @ingroup human_interface
 *  @ingroup libdaisy
 *  @brief MIDI handlers, transports, and message types
 *  @{
 */

/** @defgroup midi_events MIDI_EVENTS
 *  @{
*/

/** Simple templated wrapper for consuming sysex bytes
 * from a shared buffer/fifo without exposing write access.
 * (via ringbuffer)
 */
template <size_t MAX_SIZE = SYSEX_BUF_MAX_SIZE>
class SysexChunk
{
  public:
    enum Type : uint8_t
    {
        Invalid,
        Individual,
        SeqFirst,
        SeqIntermediate,
        SeqLast
    };

    SysexChunk()
    : type_(Type::Invalid), ringbuf_(nullptr), size_(0), bytes_read_(0)
    {
    }
    SysexChunk(Type type, RingBuffer<uint8_t, MAX_SIZE> *ringbuf, size_t size)
    : type_(type), ringbuf_(ringbuf), size_(size), bytes_read_(0)
    {
    }

    Type   GetType() const { return type_; }
    size_t GetSize() const { return size_; }
    size_t GetBytesRemaining() const { return size_ - bytes_read_; }

    /** Consume and return single byte from buffer.
     * If no more data can be read, returns 0xff */
    uint8_t ReadByte()
    {
        if(!can_read())
            return 0xff;
        bytes_read_++;
        return ringbuf_->ImmediateRead();
    }

    /** Read up to `size` bytes, returns num bytes read
     */
    size_t ReadBytes(uint8_t *buf, size_t size)
    {
        if(buf == nullptr)
            return 0;
        size_t count = 0;
        while(can_read() && count < size)
        {
            buf[count++] = ringbuf_->ImmediateRead();
            bytes_read_++;
        }
        return count;
    }

  private:
    bool can_read() const
    {
        return bytes_read_ < size_ && ringbuf_ != nullptr
               && ringbuf_->readable() > 0;
    }

    Type                           type_;
    RingBuffer<uint8_t, MAX_SIZE> *ringbuf_;
    size_t                         size_;
    size_t                         bytes_read_;
};

/** Parsed from the Status Byte, these are the common Midi Messages that can be handled. \n
At this time only 3-byte messages are correctly parsed into MidiEvents.
*/
enum MidiMessageType : uint8_t
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

enum SystemCommonType : uint8_t
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

enum SystemRealTimeType : uint8_t
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

enum ChannelModeType : uint8_t
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
    uint8_t channel;  /**< & */
    uint8_t note;     /**< & */
    uint8_t velocity; /**< & */
};


/** Struct containing note, and velocity data for a given channel.
Can be made from MidiEvent
*/
struct NoteOnEvent
{
    uint8_t channel;  /**< & */
    uint8_t note;     /**< & */
    uint8_t velocity; /**< & */
};
/** Struct containing note, and pressure data for a given channel.
Can be made from MidiEvent
*/
struct PolyphonicKeyPressureEvent
{
    uint8_t channel;
    uint8_t note;
    uint8_t pressure;
};
/** Struct containing control number, and value for a given channel.
Can be made from MidiEvent
*/
struct ControlChangeEvent
{
    uint8_t channel;        /**< & */
    uint8_t control_number; /**< & */
    uint8_t value;          /**< & */
};
/** Struct containing new program number, for a given channel.
Can be made from MidiEvent
*/
struct ProgramChangeEvent
{
    uint8_t channel; /**< & */
    uint8_t program; /**< & */
};
/** Struct containing pressure (aftertouch), for a given channel.
Can be made from MidiEvent
*/
struct ChannelPressureEvent
{
    uint8_t channel;  /**< & */
    uint8_t pressure; /**< & */
};
/** Struct containing pitch bend value for a given channel.
Can be made from MidiEvent
*/
struct PitchBendEvent
{
    uint8_t channel; /**< & */
    int16_t value;   /**< & */
};
/** Struct containing channel mode event for a given channel.
Can be made from MidiEvent
*/
struct ChannelModeEvent
{
    uint8_t         channel;    /**< & */
    ChannelModeType event_type; /**< & */
    int16_t         value;      /**< & */
};
/** Struct containing sysex data.
  Can be made from MidiEvent
  @warning Data at pointer only valid during sysex callback!
*/
struct SystemExclusiveEvent
{
    SysexChunk<> chunk;
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
    uint8_t channel; /**< & */
};
/** Struct containing ResetAllControllersEvent data.
Can be made from MidiEvent
*/
struct ResetAllControllersEvent
{
    uint8_t channel; /**< & */
    uint8_t value;   /**< & */
};
/** Struct containing LocalControlEvent data.
Can be made from MidiEvent
*/
struct LocalControlEvent
{
    uint8_t channel;           /**< & */
    bool    local_control_off; /**< & */
    bool    local_control_on;  /**< & */
};
/** Struct containing AllNotesOffEvent data.
Can be made from MidiEvent
*/
struct AllNotesOffEvent
{
    uint8_t channel; /**< & */
};
/** Struct containing OmniModeOffEvent data.
 * Can be made from MidiEvent
*/
struct OmniModeOffEvent
{
    uint8_t channel; /**< & */
};
/** Struct containing OmniModeOnEvent data.
Can be made from MidiEvent
*/
struct OmniModeOnEvent
{
    uint8_t channel; /**< & */
};
/** Struct containing MonoModeOnEvent data.
Can be made from MidiEvent
*/
struct MonoModeOnEvent
{
    uint8_t channel;      /**< & */
    uint8_t num_channels; /**< & */
};
/** Struct containing PolyModeOnEvent data.
Can be made from MidiEvent
*/
struct PolyModeOnEvent
{
    uint8_t channel; /**< & */
};


/** Simple MidiEvent with message type, channel, and data[2] members.
*/
struct MidiEvent
{
    // Newer ish.
    MidiMessageType    type;    /**< & */
    uint8_t            channel; /**< & */
    uint8_t            data[2]; /**< & */
    SysexChunk<>       sysex_chunk;
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

    ChannelModeEvent AsChannelMode()
    {
        ChannelModeEvent m;

        m.channel    = channel;
        m.event_type = (ChannelModeType)(data[0] - 120);
        m.value      = data[1];

        return m;
    }

    SystemExclusiveEvent AsSystemExclusive()
    {
        SystemExclusiveEvent m;
        // value copy is trivial, this is OK
        m.chunk = sysex_chunk;
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
        m.position = ((uint16_t)data[1] << 7) | data[0];
        return m;
    }
    SongSelectEvent AsSongSelect()
    {
        SongSelectEvent m;
        m.song = data[0];
        return m;
    }
    AllSoundOffEvent AsAllSoundOff()
    {
        AllSoundOffEvent m;
        m.channel = channel;
        return m;
    }
    ResetAllControllersEvent AsResetAllControllers()
    {
        ResetAllControllersEvent m;
        m.channel = channel;
        m.value   = data[1];
        return m;
    }
    LocalControlEvent AsLocalControl()
    {
        LocalControlEvent m;
        m.channel           = channel;
        m.local_control_off = data[1] == 0;
        m.local_control_on  = data[1] == 127;
        return m;
    }
    AllNotesOffEvent AsAllNotesOff()
    {
        AllNotesOffEvent m;
        m.channel = channel;
        return m;
    }
    OmniModeOffEvent AsOmniModeOff()
    {
        OmniModeOffEvent m;
        m.channel = channel;
        return m;
    }
    OmniModeOnEvent AsOmniModeOn()
    {
        OmniModeOnEvent m;
        m.channel = channel;
        return m;
    }
    MonoModeOnEvent AsMonoModeOn()
    {
        MonoModeOnEvent m;
        m.channel      = channel;
        m.num_channels = data[1];
        return m;
    }
    PolyModeOnEvent AsPolyModeOn()
    {
        PolyModeOnEvent m;
        m.channel = channel;
        return m;
    }

    static const char* GetTypeAsString(MidiEvent& msg)
    {
        switch(msg.type)
        {
            case NoteOff: return "NoteOff";
            case NoteOn: return "NoteOn";
            case PolyphonicKeyPressure: return "PolyKeyPres.";
            case ControlChange: return "CC";
            case ProgramChange: return "Prog. Change";
            case ChannelPressure: return "Chn. Pressure";
            case PitchBend: return "PitchBend";
            case SystemCommon: return "Sys. Common";
            case SystemRealTime: return "Sys. Realtime";
            case ChannelMode: return "Chn. Mode";
            default: return "Unknown";
        }
    }
};

/** @} */ // End midi_events

/** @} */ // End midi
} //namespace daisy
