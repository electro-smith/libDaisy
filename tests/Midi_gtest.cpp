#include <gtest/gtest.h>
#include "hid/midi.h"
#include "sys/system.h"

//get rid of compiler errors over unused args in stubs
#define UNUSED(x) (void)x

using namespace daisy;

class MidiTestTransport
{
  public:
    MidiTestTransport() {}
    ~MidiTestTransport() {}

    struct Config
    {
    };

    void Init(Config conf) { UNUSED(conf); }

    //stubs to make the handler happy
    void   StartRx(){};
    size_t Readable() { return 1; }
    void   FlushRx() {}
    void   Tx(uint8_t* buff, size_t size)
    {
        UNUSED(buff);
        UNUSED(size);
    }
    uint8_t Rx() { return 1; }
    bool    RxActive() { return true; }

  private:
};

class MidiTest : public ::testing::Test
{
  protected:
    void SetUp()
    {
        MidiHandler<MidiTestTransport>::Config conf;
        midi.Init(conf);
    }

    //help with parsing messages
    void Parse(uint8_t* msgs, int size)
    {
        for(int i = 0; i < size; i++)
        {
            midi.Parse(msgs[i]);
        }
    }

    //help with parsing messages
    MidiEvent ParseAndPop(uint8_t* msgs, int size)
    {
        Parse(msgs, size);
        return midi.PopEvent();
    }

    //help with parsing sysex messages
    MidiEvent ParseAndPopSysex(uint8_t* msgs, int size)
    {
        midi.Parse(0xf0); //sysex
        Parse(msgs, size);
        midi.Parse(0xf7); //end of sysex
        return midi.PopEvent();
    }

    //help testing
    void Test(MidiEvent event,
              uint8_t evType,
              uint8_t chkType,
              uint8_t chn,
              bool    twoData,
              uint8_t d0,
              uint8_t d1)
    {
        //test eventtype == expected type
        EXPECT_EQ(evType, chkType);

        //test channel
        EXPECT_EQ(event.channel, chn);

        //test data
        EXPECT_EQ(event.data[0], d0);
        if(twoData)
        {
            EXPECT_EQ(event.data[1], d1);
        }

        EXPECT_FALSE(midi.HasEvents());
    }


    //help test sysex
    void TestSysex(MidiEvent event, uint8_t* msgs, int size)
    {
        EXPECT_EQ((uint8_t)event.type, (uint8_t)SystemCommon);
        EXPECT_EQ((uint8_t)event.sc_type, (uint8_t)SystemExclusive);

        EXPECT_EQ(event.sysex_message_len, size);

        for(int i = 0; i < size; i++)
        {
            EXPECT_EQ(event.sysex_data[i], msgs[i]);
        }

        EXPECT_FALSE(midi.HasEvents());
    }

    MidiHandler<MidiTestTransport> midi;
};

// ================ Channel Voice Messages ================

TEST_F(MidiTest, noteOff)
{
    for(uint8_t chn = 0; chn < 16; chn++)
    {
        for(uint8_t note = 0; note < 128; note++)
        {
            for(uint8_t vel = 0; vel < 128; vel++)
            {
                uint8_t msgs[] = {(uint8_t)(0x80 + chn), note, vel};
                MidiEvent event = ParseAndPop(msgs, 3);
                NoteOffEvent offEvent = event.AsNoteOff();

                EXPECT_EQ(event.type, MidiMessageType::NoteOff);
                EXPECT_EQ(offEvent.channel, chn);
                EXPECT_EQ(offEvent.note, note);
                EXPECT_EQ(offEvent.velocity, vel);
            }
        }
    }

    EXPECT_FALSE(midi.HasEvents());
}

TEST_F(MidiTest, noteOn)
{
    for(uint8_t chn = 0; chn < 16; chn++)
    {
        for(uint8_t note = 0; note < 128; note++)
        {
            for(uint8_t vel = 0; vel < 128; vel++)
            {
                uint8_t msgs[] = {(uint8_t)(0x80 + (1 << 4) + chn), note, vel};
                MidiEvent event = ParseAndPop(msgs, 3);
                NoteOnEvent onEvent = event.AsNoteOn();

                //NoteOn of vel 0 is NoteOff
                if(vel == 0)
                    EXPECT_EQ(event.type, MidiMessageType::NoteOff);
                else    
                    EXPECT_EQ(event.type, MidiMessageType::NoteOn);
    
                EXPECT_EQ(onEvent.channel, chn);
                EXPECT_EQ(onEvent.note, note);
                EXPECT_EQ(onEvent.velocity, vel);
            }
        }
    }

    EXPECT_FALSE(midi.HasEvents());
}

TEST_F(MidiTest, polyphonicKeyPressure)
{
    for(uint8_t chn = 0; chn < 16; chn++)
    {
        for(uint8_t note = 0; note < 128; note++)
        {
            for(uint8_t pressure = 0; pressure < 128; pressure++)
            {
                uint8_t msgs[] = {(uint8_t)(0x80 + (2 << 4) + chn), note, pressure};
                MidiEvent event = ParseAndPop(msgs, 3);
                PolyphonicKeyPressureEvent pkpEvent = event.AsPolyphonicKeyPressure();

                EXPECT_EQ(event.type, MidiMessageType::PolyphonicKeyPressure);
                EXPECT_EQ(pkpEvent.channel, chn);
                EXPECT_EQ(pkpEvent.note, note);
                EXPECT_EQ(pkpEvent.pressure, pressure);
            }
        }
    }

    EXPECT_FALSE(midi.HasEvents());
}

TEST_F(MidiTest, controlChange)
{
    for(uint8_t chn = 0; chn < 16; chn++)
    {
        for(uint8_t ctrl = 0; ctrl < 120; ctrl++)
        {
            for(uint8_t val = 0; val < 128; val++)
            {
                uint8_t msgs[] = {(uint8_t)(0x80 + (3 << 4) + chn), ctrl, val};
                MidiEvent event = ParseAndPop(msgs, 3);
                ControlChangeEvent ctrlEvent = event.AsControlChange();

                EXPECT_EQ(event.type, MidiMessageType::ControlChange);
                EXPECT_EQ(ctrlEvent.channel, chn);
                EXPECT_EQ(ctrlEvent.control_number, ctrl);
                EXPECT_EQ(ctrlEvent.value, val);
            }
        }
    }

    EXPECT_FALSE(midi.HasEvents());
}

TEST_F(MidiTest, programChange)
{
    for(uint8_t chn = 0; chn < 16; chn++)
    {
        for(uint8_t prog = 0; prog < 128; prog++)
        {
            uint8_t msgs[] = {(uint8_t)(0x80 + (4 << 4) + chn), prog};
            MidiEvent event = ParseAndPop(msgs, 2);
            ProgramChangeEvent pgmEvent = event.AsProgramChange();

            EXPECT_EQ(event.type, MidiMessageType::ProgramChange);
            EXPECT_EQ(pgmEvent.channel, chn);
            EXPECT_EQ(pgmEvent.program, prog);
        }
    }

    EXPECT_FALSE(midi.HasEvents());
}

TEST_F(MidiTest, channelPressure)
{
    for(uint8_t chn = 0; chn < 16; chn++)
    {
        for(uint8_t pressure = 0; pressure < 128; pressure++)
        {
            uint8_t msgs[] = {(uint8_t)(0x80 + (5 << 4) + chn), pressure};
            MidiEvent event = ParseAndPop(msgs, 2);
            ChannelPressureEvent chpEvent = event.AsChannelPressure();

            EXPECT_EQ(event.type, MidiMessageType::ChannelPressure);
            EXPECT_EQ(chpEvent.channel, chn);
            EXPECT_EQ(chpEvent.pressure, pressure);
        }
    }

    EXPECT_FALSE(midi.HasEvents());
}

TEST_F(MidiTest, pitchBend)
{
    for(uint8_t chn = 0; chn < 16; chn++)
    {
        for(uint8_t d0 = 0; d0 < 128; d0++)
        {
            for(uint8_t d1 = 0; d1 < 128; d1++)
            {
                uint8_t msgs[] = {(uint8_t)(0x80 + (6 << 4) + chn), d0, d1};
                MidiEvent event = ParseAndPop(msgs, 3);
                PitchBendEvent pbEvent = event.AsPitchBend();

                EXPECT_EQ(event.type, MidiMessageType::PitchBend);
                EXPECT_EQ(pbEvent.channel, chn);
                EXPECT_EQ(pbEvent.value, ((uint16_t)d1 << 7) + (d0 - 8192));
            }
        }
    }

    EXPECT_FALSE(midi.HasEvents());
}

// ================ Channel Mode Messages ================ 

TEST_F(MidiTest, allSoundOff)
{
    for(uint8_t chn = 0; chn < 16; chn++)
    {
        uint8_t msg[] = {(uint8_t)(0x80 + (3 << 4) + chn), 120, 0};
        MidiEvent event = ParseAndPop(msg, 3);
        AllSoundOffEvent asoEvent = event.AsAllSoundOff();

        EXPECT_EQ((uint8_t)event.type, ChannelMode);
        EXPECT_EQ((uint8_t)event.cm_type, AllSoundOff);
        EXPECT_EQ((uint8_t)asoEvent.channel, chn);
    }

    EXPECT_FALSE(midi.HasEvents());
}

TEST_F(MidiTest, resetAllControllers)
{
    for(uint8_t chn = 0; chn < 16; chn++)
    {
        for(uint8_t val = 0; val < 128; val++){
            uint8_t msg[] = {(uint8_t)(0x80 + (3 << 4) + chn), 121, val};
            MidiEvent event = ParseAndPop(msg, 3);
            ResetAllControllersEvent racEvent = event.AsResetAllControllers();

            EXPECT_EQ((uint8_t)event.type, ChannelMode);
            EXPECT_EQ((uint8_t)event.cm_type, ResetAllControllers);
            EXPECT_EQ((uint8_t)racEvent.channel, chn);
            EXPECT_EQ((uint8_t)racEvent.value, val);
        }
    }

    EXPECT_FALSE(midi.HasEvents());
}

TEST_F(MidiTest, localControl)
{
    //Off
    for(uint8_t chn = 0; chn < 16; chn++)
    {
        uint8_t msg[] = {(uint8_t)(0x80 + (3 << 4) + chn), 122, 0};
        MidiEvent event = ParseAndPop(msg, 3);
        LocalControlEvent lcEvent = event.AsLocalControl();

        EXPECT_EQ((uint8_t)event.type, ChannelMode);
        EXPECT_EQ((uint8_t)event.cm_type, LocalControl);
        EXPECT_TRUE((uint8_t)lcEvent.local_control_off);
        EXPECT_FALSE((uint8_t)lcEvent.local_control_on);
    }

    //On
    for(uint8_t chn = 0; chn < 16; chn++)
    {
        uint8_t msg[] = {(uint8_t)(0x80 + (3 << 4) + chn), 122, 127};
        MidiEvent event = ParseAndPop(msg, 3);
        LocalControlEvent lcEvent = event.AsLocalControl();

        EXPECT_EQ((uint8_t)event.type, ChannelMode);
        EXPECT_EQ((uint8_t)event.cm_type, LocalControl);
        EXPECT_TRUE((uint8_t)lcEvent.local_control_on);
        EXPECT_FALSE((uint8_t)lcEvent.local_control_off);
    }

    EXPECT_FALSE(midi.HasEvents());
}

TEST_F(MidiTest, allNotesOff)
{
    for(uint8_t chn = 0; chn < 16; chn++){
        uint8_t msg[] = {(uint8_t)(0x80 + (3 << 4) + chn), 123, 0};
        MidiEvent event = ParseAndPop(msg, 3);
        AllNotesOffEvent anoEvent = event.AsAllNotesOff();
    
        EXPECT_EQ((uint8_t)event.type, ChannelMode);
        EXPECT_EQ((uint8_t)event.cm_type, AllNotesOff);
        EXPECT_EQ((uint8_t)anoEvent.channel, chn);
    }
    EXPECT_FALSE(midi.HasEvents());
}

TEST_F(MidiTest, omniModeOff){
    for(uint8_t chn = 0; chn < 16; chn++){
        uint8_t msg[] = {(uint8_t)(0x80 + (3 << 4) + chn), 124, 0};
        MidiEvent event = ParseAndPop(msg, 3);
        OmniModeOffEvent omoEvent = event.AsOmniModeOff();
    
        EXPECT_EQ((uint8_t)event.type, ChannelMode);
        EXPECT_EQ((uint8_t)event.cm_type, OmniModeOff);
        EXPECT_EQ((uint8_t)omoEvent.channel, chn);
    }
    EXPECT_FALSE(midi.HasEvents());
}

TEST_F(MidiTest, omniModeOn){
    for(uint8_t chn = 0; chn < 16; chn++){
        uint8_t msg[] = {(uint8_t)(0x80 + (3 << 4) + chn), 125, 0};
        MidiEvent event = ParseAndPop(msg, 3);
        OmniModeOnEvent omoEvent = event.AsOmniModeOn();
    
        EXPECT_EQ((uint8_t)event.type, ChannelMode);
        EXPECT_EQ((uint8_t)event.cm_type, OmniModeOn);
        EXPECT_EQ((uint8_t)omoEvent.channel, chn);
    }
    EXPECT_FALSE(midi.HasEvents());
}

TEST_F(MidiTest, monoModeOn){
    for(uint8_t chn = 0; chn < 16; chn++){
        for(uint8_t val = 0; val < 128; val++){
            uint8_t msg[] = {(uint8_t)(0x80 + (3 << 4) + chn), 126, val};
            MidiEvent event = ParseAndPop(msg, 3);
            MonoModeOnEvent mmoEvent = event.AsMonoModeOn();
        
            EXPECT_EQ((uint8_t)event.type, ChannelMode);
            EXPECT_EQ((uint8_t)event.cm_type, MonoModeOn);
            EXPECT_EQ((uint8_t)mmoEvent.channel, chn);
            EXPECT_EQ((uint8_t)mmoEvent.num_channels, val);
        }
    }
    EXPECT_FALSE(midi.HasEvents());
}

TEST_F(MidiTest, polyModeOn){
    for(uint8_t chn = 0; chn < 16; chn++){
        uint8_t msg[] = {(uint8_t)(0x80 + (3 << 4) + chn), 127, 0};
        MidiEvent event = ParseAndPop(msg, 3);
        PolyModeOnEvent pmoEvent = event.AsPolyModeOn();
    
        EXPECT_EQ((uint8_t)event.type, ChannelMode);
        EXPECT_EQ((uint8_t)event.cm_type, PolyModeOn);
        EXPECT_EQ((uint8_t)pmoEvent.channel, chn);
    }
    EXPECT_FALSE(midi.HasEvents());
}

//System Common Messages (no sysex)
TEST_F(MidiTest, systemCommon)
{
    //types 1-3
    for(uint8_t type = 1; type < 4; type++)
    {
        for(uint8_t d0 = 0; d0 < 128; d0++)
        {
            for(uint8_t d1 = 0; d1 < 128; d1++)
            {
                uint8_t msg[] = {uint8_t((0x0f << 4) + type), d0, d1};
                MidiEvent event = ParseAndPop(msg, 2 + (type == 2));
                Test(event,(uint8_t)event.sc_type, type, 0, type == 2, d0, d1);
            }
        }
    }

    //types 4-7
    for(uint8_t type = 4; type < 8; type++)
    {
        uint8_t msg[] = {uint8_t((0x0f << 4) + type)};
        MidiEvent event = ParseAndPop(msg, 1);
        EXPECT_EQ((uint8_t)event.sc_type, type);
    }
}

// System Real Time Messages
TEST_F(MidiTest, systemRealTime)
{
    for(uint8_t type = 0; type < 8; type++)
    {
        uint8_t msg[] = {uint8_t(0xf8 + type), 0, 0};
        MidiEvent event = ParseAndPop(msg, 1);
        EXPECT_EQ((uint8_t)event.srt_type, type);
    }
}

// sysex
TEST_F(MidiTest, systemExclusive)
{
    uint8_t msgs[135];
    for(int i = 0; i < 135; i++)
    {
        msgs[i] = (uint8_t)i;
    }

    MidiEvent event = ParseAndPopSysex(msgs, 6);
    TestSysex(event, msgs, 6);

    event = ParseAndPopSysex(msgs, 128);
    TestSysex(event, msgs, 128);

    //max len is 128, let's go past that
    event = ParseAndPopSysex(msgs, 135);
    TestSysex(event, msgs, 128);
}

//Running Status
TEST_F(MidiTest, runningStatus)
{
    //NoteOn with status bit
    uint8_t msgs[] = {0x90, 0x10, 0x0f};
    MidiEvent event = ParseAndPop(msgs, 3);
    uint8_t chkType = (uint8_t)NoteOn;
    Test(event, (uint8_t)event.type, chkType, 0, true, 0x10, 0x0f);

    //running status
    for(uint8_t i = 1; i < 20; i++)
    {
        msgs[0] = msgs[1] = i;
        MidiEvent event = ParseAndPop(msgs, 2);
        Test(event,(uint8_t)event.type, chkType, 0, true, i, i);
    }

    //Again, with Control Change, channel 3
    msgs[0] = 0xB3;
    msgs[1] = 0x10;
    msgs[2] = 0x0f;
    event = ParseAndPop(msgs, 3);
    chkType = (uint8_t)ControlChange;
    Test(event,(uint8_t)event.type, chkType, 3, true, 0x10, 0x0f);

    //running status
    for(uint8_t i = 0; i < 20; i++)
    {
        msgs[0] = msgs[1] = i;
        MidiEvent event = ParseAndPop(msgs, 2);
        Test(event,(uint8_t)event.type, chkType, 3, true, i, i);
    }
}

//Bad data
TEST_F(MidiTest, badData)
{
    //multiple msgs with status bytes in a row
    for(uint8_t i = 0; i < 20; i++)
    {
        uint8_t msg = i | 0x80;
        Parse(&msg, 1);
    }
    EXPECT_FALSE(midi.HasEvents());

    //Too short NoteOn
    uint8_t msgs[] = {0x90, 0x00};
    for(uint8_t i = 0; i < 20; i++)
    {
        msgs[1] = i;
        Parse(msgs, 2);
    }
    EXPECT_FALSE(midi.HasEvents());
}