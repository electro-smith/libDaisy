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
                    midi.Parse(0x80 + (type << 4) + chn);
                    midi.Parse(d0);

                    if(type != 4 && type != 5)
                    { //4 and 5 are just one data byte
                        midi.Parse(d1);
                    }

                    MidiEvent event = midi.PopEvent();

                    if(type == 3 && d0 > 119)
                    {
                        EXPECT_EQ((uint8_t)event.type, (uint8_t)ChannelMode);
                    }
                    else
                    {
                        EXPECT_EQ((uint8_t)event.type, type);
                    }

                    EXPECT_EQ(event.channel, chn);
                    EXPECT_EQ(event.data[0], d0);

                    if(type != 4 && type != 5)
                    { //4 and 5 are just one data byte
                        EXPECT_EQ(event.data[1], d1);
                    }
                }
            }
        }
    }
}

//Channel Mode Messages
TEST_F(MidiTest, channelMode)
{
    //All messages (misses some cases)
    for(uint8_t type = 120; type < 128; type++)
    {
        midi.Parse(0x80 + (3 << 4));
        midi.Parse(type);
        midi.Parse(0);

        MidiEvent event = midi.PopEvent();
        EXPECT_EQ((uint8_t)event.cm_type, type - 120);
    }

    //LocalControlOn
    midi.Parse(0x80 + (3 << 4));
    midi.Parse(122);
    midi.Parse(127);

    MidiEvent event = midi.PopEvent();
    EXPECT_EQ((uint8_t)event.cm_type, (uint8_t)LocalControl);
    EXPECT_EQ((uint8_t)event.data[1], 127);

    //MonoModeOn
    for(uint8_t data = 0; data < 128; data++)
    {
        midi.Parse(0x80 + (3 << 4));
        midi.Parse(126);
        midi.Parse(data);

        MidiEvent event = midi.PopEvent();
        EXPECT_EQ((uint8_t)event.cm_type, (uint8_t)MonoModeOn);
        EXPECT_EQ(event.data[1], data);
    }
}