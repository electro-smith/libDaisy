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
};
/** Struct containing ResetAllControllersEvent data.
Can be made from MidiEvent
*/
struct ResetAllControllersEvent
{
    int     channel; /**< & */
    uint8_t value;   /**< & */
};
/** Struct containing LocalControlEvent data.
Can be made from MidiEvent
*/
struct LocalControlEvent
{
    int  channel;           /**< & */
    bool local_control_off; /**< & */
    bool local_control_on;  /**< & */
};
/** Struct containing AllNotesOffEvent data.
Can be made from MidiEvent
*/
struct AllNotesOffEvent
{
    int channel; /**< & */
};
/** Struct containing OmniModeOffEvent data. 
 * Can be made from MidiEvent
*/
struct OmniModeOffEvent
{
    int channel; /**< & */
};
/** Struct containing OmniModeOnEvent data.
Can be made from MidiEvent
*/
struct OmniModeOnEvent
{
    int channel; /**< & */
};
/** Struct containing MonoModeOnEvent data.
Can be made from MidiEvent
*/
struct MonoModeOnEvent
{
    int     channel;      /**< & */
    uint8_t num_channels; /**< & */
};
/** Struct containing PolyModeOnEvent data.
Can be made from MidiEvent
*/
struct PolyModeOnEvent
{
    int channel; /**< & */
};


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
};

/** @} */ // End midi_events

/** @} */ // End midi
} //namespace daisy