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

        TestEmptyQueue();
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

        TestEmptyQueue();
    }

    //test for empty queue
    void TestEmptyQueue()
    {
        while(midi.HasEvents())
        {
            ADD_FAILURE() << "Queue not empty";
            midi.PopEvent();
        }
    }

    MidiHandler<MidiTestTransport> midi;
};

//Channel Voice messages
//for now I'm not using asNoteOff, etc. easier to just test on the raw MidiEvent
TEST_F(MidiTest, channelVoice)
{
    for(uint8_t type = 0; type < 7; type++)
    {
        for(uint8_t chn = 0; chn < 16; chn++)
        {
            for(uint8_t d0 = 0; d0 < 128; d0++)
            {
                for(uint8_t d1 = 0; d1 < 128; d1++)
                {
                    uint8_t msgs[]
                        = {(uint8_t)(0x80 + (type << 4) + chn), d0, d1};

                    bool sendThree = type != 4 && type != 5;
                    MidiEvent event = ParseAndPop(msgs, 2 + sendThree);

                    uint8_t chkType
                        = (type == 3 && d0 > 119) ? (uint8_t)ChannelMode : type;
                    Test(event, (uint8_t)event.type, chkType, chn, sendThree, d0, d1);
                }
            }
        }
    }
}

//Channel Mode Messages
// also doesn't totally test channels (for now)
TEST_F(MidiTest, channelMode)
{
    //All messages
    for(uint8_t type = 120; type < 128; type++)
    {
        uint8_t msg[] = {0x80 + (3 << 4), type, 0};
        MidiEvent event = ParseAndPop(msg, 3);
        EXPECT_EQ((uint8_t)event.cm_type, type - 120);
    }

    //LocalControlOn
    uint8_t msg[] = {0x80 + (3 << 4), 122, 127};
    MidiEvent event = ParseAndPop(msg, 3);
    Test(event, (uint8_t)event.cm_type, (uint8_t)LocalControl, 0, true, 122, 127);

    //MonoModeOn
    for(uint8_t data = 0; data < 128; data++)
    {
        uint8_t msg[] = {0x80 + (3 << 4), 126, data};
        MidiEvent event = ParseAndPop(msg, 3);
        Test(event,(uint8_t)event.cm_type, (uint8_t)MonoModeOn, 0, true, 126, data);
    }
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
    for(uint8_t i = 0; i < 20; i++)
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
    TestEmptyQueue();

    //Too short NoteOn
    uint8_t msgs[] = {0x90, 0x00};
    for(uint8_t i = 0; i < 20; i++)
    {
        msgs[1] = i;
        Parse(msgs, 2);
    }
    TestEmptyQueue();
}