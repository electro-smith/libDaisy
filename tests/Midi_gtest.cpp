#include "hid/midi.h"
#include <gtest/gtest.h>

using namespace daisy;

class MidiTestTransport{
    public:
    MidiTestTransport() {}
    ~MidiTestTransport() {}

    struct Config {};
    void Init(Config conf) {};
}

TEST(util_MidiHandler, a_stateAfterInit){
    MidiHandler<MidiTestTransport> midi;
    MidiHandler<MidiTestTransport>::Config conf;
    
    midi.Init(conf);
    EXPECT_TRUE(true);
}