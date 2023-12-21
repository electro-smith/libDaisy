/** Example of multichannel audio with two SAIs configured for 
 *  i2s codecs
 * 
 *  This example is based on the seed2 DFM hardware.
 *  Both codecs are PCM3060
 */
#include "daisy_seed.h"

/** This prevents us from having to type "daisy::" in front of a lot of things. */
using namespace daisy;

/** Global Hardware access */
DaisySeed hw;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        out[0][i] = in[0][i];
        out[1][i] = in[1][i];
        out[2][i] = in[2][i];
        out[3][i] = in[3][i];
    }
}

int main(void)
{
    /** Initialize our hardware */
    hw.Init();

    /** Configure the SAI2 peripheral for our secondary codec. */
    SaiHandle         external_sai_handle;
    SaiHandle::Config external_sai_cfg;
    external_sai_cfg.periph          = SaiHandle::Config::Peripheral::SAI_2;
    external_sai_cfg.sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    external_sai_cfg.bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    external_sai_cfg.a_sync          = SaiHandle::Config::Sync::SLAVE;
    external_sai_cfg.b_sync          = SaiHandle::Config::Sync::MASTER;
    external_sai_cfg.a_dir           = SaiHandle::Config::Direction::TRANSMIT;
    external_sai_cfg.b_dir           = SaiHandle::Config::Direction::RECEIVE;
    external_sai_cfg.pin_config.fs   = seed::D27;
    external_sai_cfg.pin_config.mclk = seed::D24;
    external_sai_cfg.pin_config.sck  = seed::D28;
    external_sai_cfg.pin_config.sb   = seed::D25;
    external_sai_cfg.pin_config.sa   = seed::D26;

    /** Initialize the SAI new handle */
    external_sai_handle.Init(external_sai_cfg);

    /** Reconfigure Audio for two codecs 
     * 
     *  Default eurorack circuit has an extra 6dB headroom 
     *  so the 0.5 here makes it so that a -1 to 1 audio signal
     *  will correspond to a -5V to 5V (10Vpp) audio signal.
     *  Audio will clip at -2 to 2, and result 20Vpp output.
     */
    AudioHandle::Config audio_cfg;
    audio_cfg.blocksize  = 48;
    audio_cfg.samplerate = SaiHandle::Config::SampleRate::SAI_48KHZ;
    audio_cfg.postgain   = 0.5f;

    /** Initialize for two SAIs, including the built-in SAI that is 
     *  configured during hw.Init()
     */
    hw.audio_handle.Init(audio_cfg, hw.AudioSaiHandle(), external_sai_handle);

    /** Finally start the audio */
    hw.StartAudio(AudioCallback);

    /** Infinite Loop */
    while(1) {}
}