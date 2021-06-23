#include <gtest/gtest.h>
#include "hid/midi.h"

//get rid of compiler errors over unused args in stubs
#define UNUSED(x) (void)x
#define MAX_MESSAGES 3000

using namespace daisy;

class MidiTestTransport{
    public:
    MidiTestTransport() {}
    ~MidiTestTransport() {}

    struct Config {};

    void Init(Config conf) { UNUSED(conf); }

    //stubs to make the handler happy
    void StartRx() {};
    size_t Readable() { return 1; }
    void FlushRx() { }
    void Tx(uint8_t* buff, size_t size) { UNUSED(buff); UNUSED(size); }
    uint8_t Rx() { return 1; }    
    bool RxActive() { return true; }

    private:
};

TEST(util_MidiHandler, a_stateAfterInit){
    MidiHandler<MidiTestTransport> midi;
    MidiHandler<MidiTestTransport>::Config handler_conf;
    midi.Init(handler_conf);
    
    midi.Parse(0x80);
    midi.Parse(0x00);
    midi.Parse(0x00);

    MidiEvent event = midi.PopEvent();
    EXPECT_TRUE(event.type == NoteOff);

    NoteOffEvent no_event = event.AsNoteOff();
    EXPECT_EQ(no_event.channel, 0);
    EXPECT_EQ(no_event.note, 0);
    EXPECT_EQ(no_event.velocity, 0);
}    