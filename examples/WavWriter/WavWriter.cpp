/** Generation of a simple Audio signal */
#include "daisy_seed.h"
#include <cmath>

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

static constexpr float  kTargetSr     = 48000.f;
static constexpr size_t kTransferSize = 16384;

/** Global Hardware access */
DaisySeed                hw;
SdmmcHandler             sdmmc;
FatFSInterface           fsi;
WavWriter<kTransferSize> wav_writer;

/** Basic Fixed-frequency oscillator */
struct SimpleOsc
{
    static constexpr float kTargetFreq = 220.f;
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

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

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
    hw.SetLed(true);

    /** Set up WAV File */
    WavWriter<kTransferSize>::Config cfg;
    cfg.bitspersample = 16;
    cfg.channels      = 2;
    cfg.samplerate    = kTargetSr;
    wav_writer.Init(cfg);

    /** Prepare to record a 1s 220Hz Audio File */
    SimpleOsc oscillator;
    size_t    duration_sec      = 1;
    size_t    duration_in_samps = duration_sec * kTargetSr;

    wav_writer.OpenFile("ExampleWavFile.wav");
    for(size_t i = 0; i < duration_in_samps; i++)
    {
        // If recording Realtime Audio:
        // The rendering/sampling should occur in the realtime audio interrupt
        float sample            = oscillator.RenderSample();
        float samps_to_write[2] = {sample, sample};
        wav_writer.Sample(samps_to_write);

        // The actual DiskIO should happen outside of the realtime audio interrupt
        // For offline-rendering, it is okay to do this check on every sample.
        wav_writer.Write();
    }
    // Flush and Close
    wav_writer.SaveFile();

    while(1)
    {
        // Blink Afterwards to show success
        hw.SetLed((System::GetNow() & 511) > 255);
    }
}