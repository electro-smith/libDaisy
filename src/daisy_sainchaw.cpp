#include "daisy_sainchaw.h"
#include "dev/codec_ak4556.h"

using namespace daisy;

// Hardware Definitions
#define PIN_ENC_CLICK 0
#define PIN_ENC_B 11
#define PIN_ENC_A 12
#define NOTE_LED_PIN 4 // this is the Daisy pin (not MCU pin)
#define PITCH_LED_PIN 3 // this is the Daisy pin (not MCU pin)
#define NORMALIZATION_PROBE_PIN 21 // this is the Daisy pin (not MCU pin)

#define PIN_AK4556_RESET 29

#define PIN_DETUNE_CTRL 15
#define PIN_SHAPE_CTRL 16
#define PIN_FM_CTRL 17
#define PIN_PITCH_1_CTRL 18
#define PIN_PITCH_2_CTRL 19
#define PIN_PITCH_3_CTRL 21

void Sainchaw::Init(bool boost)
{
    // Configure Seed first
    seed.Configure();
    seed.Init(boost);
    InitAudio();
    InitEncoder();
    InitControls();

    // LEDs
    note_led.pin   = seed.GetPin(NOTE_LED_PIN);
    note_led.mode  = DSY_GPIO_MODE_OUTPUT_PP;
    // note_led.pull  = DSY_GPIO_NOPULL;
    dsy_gpio_init(&note_led);
    alt_led.pin  = seed.GetPin(PITCH_LED_PIN);
    alt_led.mode = DSY_GPIO_MODE_OUTPUT_PP;
    // pitch_led.pull  = DSY_GPIO_NOPULL;
    dsy_gpio_init(&alt_led);
    normalization_probe.pin  = seed.GetPin(NORMALIZATION_PROBE_PIN);
    normalization_probe.mode = DSY_GPIO_MODE_OUTPUT_PP;
    // pitch_led.pull  = DSY_GPIO_NOPULL;
    dsy_gpio_init(&alt_led);


    // Set Screen update vars
    screen_update_period_ = 17; // roughly 60Hz
    screen_update_last_   = seed.system.GetNow();
}

void Sainchaw::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void Sainchaw::SetHidUpdateRates()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].SetSampleRate(AudioCallbackRate());
    }
}

void Sainchaw::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void Sainchaw::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void Sainchaw::StopAudio()
{
    seed.StopAudio();
}

void Sainchaw::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float Sainchaw::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void Sainchaw::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t Sainchaw::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float Sainchaw::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void Sainchaw::StartAdc()
{
    seed.adc.Start();
}

/** Stops Transfering data from the ADC */
void Sainchaw::StopAdc()
{
    seed.adc.Stop();
}


void Sainchaw::ProcessAnalogControls()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].Process();
    }
}
float Sainchaw::GetKnobValue(Ctrl k)
{
    return (controls[k].Value());
}

void Sainchaw::ProcessDigitalControls()
{
    encoder.Debounce();
}

// Private Function Implementations
// set SAI2 stuff -- run this between seed configure and init
void Sainchaw::InitAudio()
{
    // Handle Seed Audio as-is and then
    SaiHandle::Config sai_config[2];
    // Internal Codec
    if(seed.CheckBoardVersion() == DaisySeed::BoardVersion::DAISY_SEED_1_1)
    {
        sai_config[0].pin_config.sa = {DSY_GPIOE, 6};
        sai_config[0].pin_config.sb = {DSY_GPIOE, 3};
        sai_config[0].a_dir         = SaiHandle::Config::Direction::RECEIVE;
        sai_config[0].b_dir         = SaiHandle::Config::Direction::TRANSMIT;
    }
    else
    {
        sai_config[0].pin_config.sa = {DSY_GPIOE, 6};
        sai_config[0].pin_config.sb = {DSY_GPIOE, 3};
        sai_config[0].a_dir         = SaiHandle::Config::Direction::TRANSMIT;
        sai_config[0].b_dir         = SaiHandle::Config::Direction::RECEIVE;
    }
    sai_config[0].periph          = SaiHandle::Config::Peripheral::SAI_1;
    sai_config[0].sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config[0].bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config[0].a_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config[0].b_sync          = SaiHandle::Config::Sync::SLAVE;
    sai_config[0].pin_config.fs   = {DSY_GPIOE, 4};
    sai_config[0].pin_config.mclk = {DSY_GPIOE, 2};
    sai_config[0].pin_config.sck  = {DSY_GPIOE, 5};

    // External Codec
    sai_config[1].periph          = SaiHandle::Config::Peripheral::SAI_2;
    sai_config[1].sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config[1].bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config[1].a_sync          = SaiHandle::Config::Sync::SLAVE;
    sai_config[1].b_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config[1].a_dir           = SaiHandle::Config::Direction::TRANSMIT;
    sai_config[1].b_dir           = SaiHandle::Config::Direction::RECEIVE;
    sai_config[1].pin_config.fs   = seed.GetPin(27);
    sai_config[1].pin_config.mclk = seed.GetPin(24);
    sai_config[1].pin_config.sck  = seed.GetPin(28);
    sai_config[1].pin_config.sb   = seed.GetPin(25);
    sai_config[1].pin_config.sa   = seed.GetPin(26);

    SaiHandle sai_handle[2];
    sai_handle[0].Init(sai_config[0]);
    sai_handle[1].Init(sai_config[1]);

    // Reset Pin for AK4556
    // Built-in AK4556 was reset during Seed Init
    dsy_gpio_pin codec_reset_pin = seed.GetPin(PIN_AK4556_RESET);
    Ak4556::Init(codec_reset_pin);

    // Reinit Audio for _both_ codecs...
    AudioHandle::Config cfg;
    cfg.blocksize  = 48;
    cfg.samplerate = SaiHandle::Config::SampleRate::SAI_48KHZ;
    cfg.postgain   = 0.5f;
    seed.audio_handle.Init(cfg, sai_handle[0], sai_handle[1]);
}

void Sainchaw::InitControls()
{
    AdcChannelConfig cfg[CTRL_LAST];

    // Init ADC channels with Pins
    cfg[DETUNE_CTRL].InitSingle(seed.GetPin(PIN_DETUNE_CTRL));
    cfg[SHAPE_CTRL].InitSingle(seed.GetPin(PIN_SHAPE_CTRL));
    cfg[FM_CTRL].InitSingle(seed.GetPin(PIN_FM_CTRL));
    cfg[PITCH_1_CTRL].InitSingle(seed.GetPin(PIN_PITCH_1_CTRL));
    cfg[PITCH_2_CTRL].InitSingle(seed.GetPin(PIN_PITCH_2_CTRL));
    cfg[PITCH_3_CTRL].InitSingle(seed.GetPin(PIN_PITCH_3_CTRL));

    // Initialize ADC
    seed.adc.Init(cfg, CTRL_LAST);

    // Initialize AnalogControls, with flip set to true
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].Init(seed.adc.GetPtr(i), AudioCallbackRate(), true);
    }
}

void Sainchaw::InitEncoder()
{
    encoder.Init(seed.GetPin(PIN_ENC_A),
                 seed.GetPin(PIN_ENC_B),
                 seed.GetPin(PIN_ENC_CLICK));
}

void Sainchaw::SetNoteLed(bool state) 
{
     dsy_gpio_write(&note_led, state);
}

void Sainchaw::SetAltLed(bool state) 
{
     dsy_gpio_write(&note_led, state);
}
