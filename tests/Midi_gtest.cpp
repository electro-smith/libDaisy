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
    void ParseAndPop(uint8_t* msgs, uint8_t size)
    {
        for(uint8_t i = 0; i < size; i++)
        {
            midi.Parse(msgs[i]);
        }
        event = midi.PopEvent();
    }

    //help testing
    void Test(uint8_t evType,
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
    }

    MidiHandler<MidiTestTransport> midi;
    MidiEvent                      event;
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
                    ParseAndPop(msgs, 2 + sendThree);

                    uint8_t chkType
                        = (type == 3 && d0 > 119) ? (uint8_t)ChannelMode : type;
                    Test((uint8_t)event.type, chkType, chn, sendThree, d0, d1);
                }
            }
        }
    }
}

//Channel Mode Messages
TEST_F(MidiTest, channelMode)
{
    //All messages
    for(uint8_t type = 120; type < 128; type++)
    {
        uint8_t msg[] = {0x80 + (3 << 4), type, 0};
        ParseAndPop(msg, 3);
        EXPECT_EQ((uint8_t)event.cm_type, type - 120);
    }

    //LocalControlOn
    uint8_t msg[] = {0x80 + (3 << 4), 122, 127};
    ParseAndPop(msg, 3);
    Test((uint8_t)event.cm_type, (uint8_t)LocalControl, 0, true, 122, 127);

    //MonoModeOn
    for(uint8_t data = 0; data < 128; data++)
    {
        uint8_t msg[] = {0x80 + (3 << 4), 126, data};
        ParseAndPop(msg, 3);
        Test((uint8_t)event.cm_type, (uint8_t)MonoModeOn, 0, true, 126, data);
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
                ParseAndPop(msg, 2 + (type == 2));
                Test((uint8_t)event.sc_type, type, 0, type == 2, d0, d1);
            }
        }
    }

    //types 4-7
    for(uint8_t type = 4; type < 8; type++)
    {
        uint8_t msg[] = {uint8_t((0x0f << 4) + type)};
        ParseAndPop(msg, 1);
        EXPECT_EQ((uint8_t)event.sc_type, type);
    }
}