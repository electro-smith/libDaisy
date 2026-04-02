/** Demonstration of using the WavWriter to record a file from realtime audio
 *
 *  The Program is built to be portable, and only require an SD Card connection.
 *
 *  The program will bootup, initialize hardware, and then sit for the pre-determined
 *  delay set by kStartupDelayMs with the USR LED set to on.
 *  After this delay, the recording will start and be made of two generated sine waves.
 *  The recording will be 2 seconds long, and recorded at 48kHz 16-bit.
 *  During recording the LED will blink fast (every 128ms)
 *
 *  After the two second recording period, the state will transition to finalizing the
 *  file, which should occur quickly, and does not do any changes to the LED.
 *  Once the file is finalized, the LED will begin to blink slowly (every 512 ms)
 *
 *  The various state transitions are automatic here, but could be adapted to be based
 *  on user input via button presses, or more complex UI interactions.
 */
#include "daisy_seed.h"
#include <cmath>

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

static constexpr const float    kTargetSr          = 48000.f;
static constexpr const size_t   kTransferSize      = 16384;
static constexpr const size_t   kFileDurationSamps = 96000; //< 2s @ 48kHz
static constexpr const uint32_t kStartupDelayMs    = 2000;

/** Generic Fixed-frequency oscillator */
template <int freq>
struct SimpleOsc
{
    static constexpr float kTargetFreq = static_cast<float>(freq);
    static constexpr float kSignalIncrement
        = (M_TWOPI * kTargetFreq) * (1.f / kTargetSr);
    float phs_;
    SimpleOsc() : phs_(0.f) {}

    inline float RenderSample()
    {
        float signal = sin(phs_) * 0.5f;
        phs_ += kSignalIncrement;
        if(phs_ > M_TWOPI)
            phs_ -= M_TWOPI;
        return signal;
    }
};

/** Enum for tracking the state of our example process
 *
 *  We will begin in "startup" for a set duration, prior to recording.
 *  Then transition to recording, where the WAV file will be recorded from audio
 *  in the real time engine.
 *  After the duration of the recording has passed, we'll finalize the WAV file,
 *  and update to Done.
 */
enum class State
{
    Startup,
    Recording,
    Finalize,
    Done,
};

/** Global Hardware access */
DaisySeed      hw;
SdmmcHandler   sdmmc;
FatFSInterface fsi;

/** Globals used for recording our WAV file */
WavWriter<kTransferSize> wav_writer;
State                    state = State::Startup;
SimpleOsc<220>           osc1;
SimpleOsc<330>           osc2;
size_t                   duration_counter = 0;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    switch(state)
    {
        case State::Recording:
            for(size_t i = 0; i < size; i++)
            {
                float samples[2] = {
                    osc1.RenderSample(),
                    osc2.RenderSample(),
                };
                wav_writer.Sample(samples);
                duration_counter++;
                if(duration_counter > kFileDurationSamps)
                {
                    state = State::Finalize;
                }
                out[0][i] = samples[0];
                out[1][i] = samples[1];
            }
            break;
        default:
            /** Silence on the outputs when not recording */
            std::fill(out[0], out[0] + size, 0.f);
            std::fill(out[1], out[1] + size, 0.f);
            break;
    }
}


int main(void)
{
    /** Initialize our hardware */
    hw.Init();
    state = State::Startup;

    /** Set up SD Card */
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    sd_cfg.width = SdmmcHandler::BusWidth::BITS_1;
    sdmmc.Init(sd_cfg);
    FatFSInterface::Config fsi_cfg;
    fsi_cfg.media = FatFSInterface::Config::MEDIA_SD;
    fsi.Init(fsi_cfg);
    if(f_mount(&fsi.GetSDFileSystem(), "/", 0) != FR_OK)
    {
        while(1)
        {
            hw.SetLed((System::GetNow() & 127) > 63);
        }
    }

    /** Init WAV File */
    WavWriter<kTransferSize>::Config cfg;
    cfg.bitspersample = 16;
    cfg.channels      = 2;
    cfg.samplerate    = kTargetSr;
    wav_writer.Init(cfg);


    /** Startup the Audio Engine */
    hw.StartAudio(AudioCallback);


    uint32_t startup_time = System::GetNow();


    while(1)
    {
        auto now = daisy::System::GetNow();

        /** Manage State */
        switch(state)
        {
            case State::Startup:
                if(now - startup_time > kStartupDelayMs)
                {
                    wav_writer.OpenFile("RealtimeWavFile.wav");
                    state = State::Recording;
                }
                // LED is solid during startup
                hw.SetLed(true);
                break;
            case State::Recording:
                // Check status of WAV file writing, and manage Disk I/O.
                // Audio callback will convert state to Finalize when it has
                // written the duration of files.
                wav_writer.Write();

                // Blink fast during recording:
                hw.SetLed((System::GetNow() & 127) > 63);
                break;
            case State::Finalize:
                // After the file is complete, we flush any remaining samples, and
                // update the WAV headers, etc.
                wav_writer.SaveFile();
                state = State::Done;
                // No LED activity for this state because its a blocking operation,
                // and should complete pretty quickly.
                break;
            case State::Done:
                // Blink Slowly to show success
                hw.SetLed((System::GetNow() & 1023) > 511);
                break;
        }
    }
}