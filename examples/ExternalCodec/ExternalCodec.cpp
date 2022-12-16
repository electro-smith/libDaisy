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

    /** Configuration for both SAI peripherals */
    SaiHandle::Config sai_config[2];

    /** This is setup for the Daisy Seed 2 DFM, (or the rev4 Daisy Seed) */
    sai_config[0].pin_config.sa   = Pin(PORTE, 6);
    sai_config[0].pin_config.sb   = Pin(PORTE, 3); //{DSY_GPIOE, 3};
    sai_config[0].pin_config.fs   = Pin(PORTE, 4); //{DSY_GPIOE, 4};
    sai_config[0].pin_config.mclk = Pin(PORTE, 2); //{DSY_GPIOE, 2};
    sai_config[0].pin_config.sck  = Pin(PORTE, 5); //{DSY_GPIOE, 5};
    sai_config[0].a_dir           = SaiHandle::Config::Direction::TRANSMIT;
    sai_config[0].b_dir           = SaiHandle::Config::Direction::RECEIVE;
    sai_config[0].periph          = SaiHandle::Config::Peripheral::SAI_1;
    sai_config[0].sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config[0].bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config[0].a_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config[0].b_sync          = SaiHandle::Config::Sync::SLAVE;

    /** Configure the SAI2 peripheral for our secondary codec. */
    sai_config[1].periph          = SaiHandle::Config::Peripheral::SAI_2;
    sai_config[1].sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config[1].bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config[1].a_sync          = SaiHandle::Config::Sync::SLAVE;
    sai_config[1].b_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config[1].a_dir           = SaiHandle::Config::Direction::TRANSMIT;
    sai_config[1].b_dir           = SaiHandle::Config::Direction::RECEIVE;
    sai_config[1].pin_config.fs   = seed::D27;
    sai_config[1].pin_config.mclk = seed::D24;
    sai_config[1].pin_config.sck  = seed::D28;
    sai_config[1].pin_config.sb   = seed::D25;
    sai_config[1].pin_config.sa   = seed::D26;

    /** Initialize the SAI handles */
    SaiHandle sai_handle[2];
    sai_handle[0].Init(sai_config[0]);
    sai_handle[1].Init(sai_config[1]);

    /** Flick the PCM3060 Deemphasis pin on the Daisy Seed2 DFM 
     *  TODO: add this to the seed Init (it should only have to happen once period.)
     */
    GPIO deemp;
    Pin  deemp_pin(PORTB, 11);
    deemp.Init(deemp_pin, GPIO::Mode::OUTPUT);
    deemp.Write(0);

    AudioHandle::Config audio_cfg;
    audio_cfg.blocksize  = 48;
    audio_cfg.samplerate = SaiHandle::Config::SampleRate::SAI_48KHZ;
    /** Default eurorack circuit has an extra 6dB headroom 
     *  so the 0.5 here makes it so that a -1 to 1 audio signal
     *  will correspond to a -5V to 5V (10Vpp) audio signal.
     *  Audio will clip at -2 to 2, and result 20Vpp output.
     */
    audio_cfg.postgain = 0.5f;
    hw.audio_handle.Init(audio_cfg, sai_handle[0], sai_handle[1]);


    /** Finally start the audio */
    hw.StartAudio(AudioCallback);

    /** Infinite Loop */
    while(1) {}
}