/** Generation of a simple Audio signal */
#include "daisy_seed.h"
#include <cmath>

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

// static constexpr size_t kTransferSize = 65536;
// static constexpr size_t kTransferSize = 32767;
// static constexpr size_t kTransferSize = 16384;
// static constexpr size_t kTransferSize = 8192;
static constexpr size_t kTransferSize = 4096;
// static constexpr size_t kTransferSize = 2048;

/** Global Hardware access */
DaisySeed                      hw;
SdmmcHandler                   sdmmc;
FatFSInterface                 fsi;
temp::WavPlayer<kTransferSize> player;
temp::FileTable<8>             file_table;


/** Temp stuff for testing: */
Switch         btn1, btn2;
Encoder        enc;
MidiUsbHandler midi;

int selected_file = 0;
int open_file     = 0;


/** Extra temp stuff for playback speed.. */
// const float kPlaybackSpeedOptions[]
//     = {0.25f, 0.5f, 0.75f, 1.f, 1.5f, 2.0f, 3.f, 4.f};
// const float kPlaybackNotesOptions[]
//     = {-24.f, -12.f, -5.f, 0.f, 4.f, 7.f, 12.f, 24.f};
// int playback_speed_idx;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    btn1.Debounce();
    btn2.Debounce();
    enc.Debounce();
    if(btn1.FallingEdge())
    {
        player.SetPlaying(!player.GetPlaying());
    }
    if(btn2.FallingEdge())
    {
        player.SetLooping(!player.GetLooping());
    }
    if(enc.FallingEdge())
    {
        player.Restart();
    }

    int inc = enc.Increment();
    if(inc != 0)
    {
        // Increment the file selection
        auto num_files = file_table.GetNumFiles();
        selected_file += inc;
        if(selected_file > (int)num_files - 1)
            selected_file = num_files - 1;
        else if(selected_file < 0)
            selected_file = 0;
    }

    for(size_t i = 0; i < size; i++)
    {
        float samps[2];
        player.Stream(samps, 2);
        out[0][i] = samps[0];
        out[1][i] = samps[1];
    }
}

int main(void)
{
    /** Initialize our hardware */
    hw.Init(true);

    /** Daisy Pod Buttons/Encoder for some basic controls */
    btn1.Init(seed::D27);
    btn2.Init(seed::D28);
    enc.Init(seed::D26, seed::D25, seed::D13);

    /** USB Midi for testing a few things */
    MidiUsbHandler::Config midi_cfg;
    midi_cfg.transport_config.periph
        = MidiUsbTransport::Config::Periph::INTERNAL;
    midi.Init(midi_cfg);

    /** The SD Card/FatFS Initialization remains unchanged */
    SdmmcHandler::Config sdcfg;
    sdcfg.Defaults();
    sdcfg.speed = SdmmcHandler::Speed::VERY_FAST;
    sdcfg.width = SdmmcHandler::BusWidth::BITS_4;
    sdmmc.Init(sdcfg);
    fsi.Init(FatFSInterface::Config::Media::MEDIA_SD);
    f_mount(&fsi.GetSDFileSystem(), "/", 1);

    /** With the old, multi-file API */
    // player.Init("/");
    // player.Open(0);

    /** New API with more flexible multi-file support
     *  If you don't need to playback multiple files, you can use:
     *  player.Init("myfile.wav");
     *
     *  The FileTable object just provides access to some additional
     *  info like file name, etc. that can be useful when building
     *  UIs.
     */
    file_table.Fill("/", ".wav");
    selected_file = 0;
    if(file_table.GetNumFiles() > 0)
        player.Init(file_table.GetFileName(selected_file));

    /** Start the processes */
    hw.StartAudio(AudioCallback);
    midi.StartReceive();

    /** Timing variables for MIDI Output timing */
    uint32_t last_midi_time;
    last_midi_time = System::GetNow();

    while(1)
    {
        /** This does the actual Disk I/O whenever the Audio FIFOs are low */
        player.Prepare();

        /** On Board LED indicates Looping State */
        hw.SetLed(player.GetLooping());

        /** Open a new file whenever the encoder selects a new file */
        if(open_file != selected_file)
        {
            player.Open(file_table.GetFileName(selected_file));
            open_file = selected_file;
        }

        /** Handle MIDI Input */
        midi.Listen();
        while(midi.HasEvents())
        {
            auto msg = midi.PopEvent();
            switch(msg.type)
            {
                case NoteOn:
                {
                    float nn = msg.AsNoteOn().note - 36.f;
                    if(nn < -36.f)
                        nn = -36.f;
                    else if(nn > 48.f)
                        nn = 48.f;
                    player.SetPlaybackSpeedSemitones(nn);
                    player.Restart();
                    if(!player.GetPlaying())
                        player.SetPlaying(true);
                }
                break;
                default: break;
            }
        }

        /** Output the position of the file playback as MIDI CC1 at 500Hz
         */
        auto now = System::GetNow();
        if(now - last_midi_time > 2)
        {
            float   pos     = player.GetNormalizedPosition();
            uint8_t bytes[] = {0xb0, 0x01, static_cast<uint8_t>(pos * 127.f)};
            midi.SendMessage(bytes, 3);
            last_midi_time = now;
        }
    }
}