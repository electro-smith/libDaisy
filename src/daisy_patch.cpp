#include "daisy_patch.h"
#include "dev/codec_ak4556.h"

using namespace daisy;

// Hardware Definitions
constexpr Pin PIN_ENC_CLICK  = seed::D0;
constexpr Pin PIN_ENC_B      = seed::D11;
constexpr Pin PIN_ENC_A      = seed::D12;
constexpr Pin PIN_OLED_DC    = seed::D9;
constexpr Pin PIN_OLED_RESET = seed::D30;
constexpr Pin PIN_MIDI_OUT   = seed::D13;
constexpr Pin PIN_MIDI_IN    = seed::D14;
constexpr Pin PIN_GATE_OUT   = seed::D17;
constexpr Pin PIN_GATE_IN_1  = seed::D20;
constexpr Pin PIN_GATE_IN_2  = seed::D19;
constexpr Pin PIN_SAI_SCK_A  = seed::D28;
constexpr Pin PIN_SAI2_FS_A  = seed::D27;
constexpr Pin PIN_SAI2_SD_A  = seed::D26;
constexpr Pin PIN_SAI2_SD_B  = seed::D25;
constexpr Pin PIN_SAI2_MCLK  = seed::D24;

constexpr Pin PIN_AK4556_RESET = seed::D29;

constexpr Pin PIN_CTRL_1 = seed::D15;
constexpr Pin PIN_CTRL_2 = seed::D16;
constexpr Pin PIN_CTRL_3 = seed::D21;
constexpr Pin PIN_CTRL_4 = seed::D18;


void DaisyPatch::Init(bool boost)
{
    // Configure Seed first
    seed.Configure();
    seed.Init(boost);
    InitAudio();
    InitDisplay();
    InitCvOutputs();
    InitEncoder();
    InitGates();
    InitDisplay();
    InitMidi();
    InitControls();
    // Set Screen update vars
    screen_update_period_ = 17; // roughly 60Hz
    screen_update_last_   = seed.system.GetNow();
}

void DaisyPatch::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisyPatch::SetHidUpdateRates()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].SetSampleRate(AudioCallbackRate());
    }
}

void DaisyPatch::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyPatch::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyPatch::StopAudio()
{
    seed.StopAudio();
}

void DaisyPatch::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float DaisyPatch::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

void DaisyPatch::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisyPatch::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

float DaisyPatch::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisyPatch::StartAdc()
{
    seed.adc.Start();
}

/** Stops Transfering data from the ADC */
void DaisyPatch::StopAdc()
{
    seed.adc.Stop();
}


void DaisyPatch::ProcessAnalogControls()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].Process();
    }
}
float DaisyPatch::GetKnobValue(Ctrl k)
{
    return (controls[k].Value());
}

void DaisyPatch::ProcessDigitalControls()
{
    encoder.Debounce();
}

// This will render the display with the controls as vertical bars
void DaisyPatch::DisplayControls(bool invert)
{
    bool on, off;
    on  = invert ? false : true;
    off = invert ? true : false;
    if(seed.system.GetNow() - screen_update_last_ > screen_update_period_)
    {
        // Graph Knobs
        size_t barwidth, barspacing;
        size_t curx, cury;
        screen_update_last_ = seed.system.GetNow();
        barwidth            = 15;
        barspacing          = 20;
        display.Fill(off);
        // Bars for all four knobs.
        for(size_t i = 0; i < DaisyPatch::CTRL_LAST; i++)
        {
            float  v;
            size_t dest;
            curx = (barspacing * i + 1) + (barwidth * i);
            cury = display.Height();
            v    = GetKnobValue(static_cast<DaisyPatch::Ctrl>(i));
            dest = (v * display.Height());
            for(size_t j = dest; j > 0; j--)
            {
                for(size_t k = 0; k < barwidth; k++)
                {
                    display.DrawPixel(curx + k, cury - j, on);
                }
            }
        }
        display.Update();
    }
}

// Private Function Implementations
// set SAI2 stuff -- run this between seed configure and init
void DaisyPatch::InitAudio()
{
    // Handle Seed Audio as-is and then
    SaiHandle::Config sai_config[2];
    // Internal Codec
    if(seed.CheckBoardVersion() == DaisySeed::BoardVersion::DAISY_SEED_1_1)
    {
        sai_config[0].pin_config.sa = Pin(PORTE, 6);
        sai_config[0].pin_config.sb = Pin(PORTE, 3);
        sai_config[0].a_dir         = SaiHandle::Config::Direction::RECEIVE;
        sai_config[0].b_dir         = SaiHandle::Config::Direction::TRANSMIT;
    }
    else
    {
        sai_config[0].pin_config.sa = Pin(PORTE, 6);
        sai_config[0].pin_config.sb = Pin(PORTE, 3);
        sai_config[0].a_dir         = SaiHandle::Config::Direction::TRANSMIT;
        sai_config[0].b_dir         = SaiHandle::Config::Direction::RECEIVE;
    }
    sai_config[0].periph          = SaiHandle::Config::Peripheral::SAI_1;
    sai_config[0].sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config[0].bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config[0].a_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config[0].b_sync          = SaiHandle::Config::Sync::SLAVE;
    sai_config[0].pin_config.fs   = Pin(PORTE, 4);
    sai_config[0].pin_config.mclk = Pin(PORTE, 2);
    sai_config[0].pin_config.sck  = Pin(PORTE, 5);

    // External Codec
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

    SaiHandle sai_handle[2];
    sai_handle[0].Init(sai_config[0]);
    sai_handle[1].Init(sai_config[1]);

    // Reset Pin for AK4556
    // Built-in AK4556 was reset during Seed Init
    dsy_gpio_pin codec_reset_pin = PIN_AK4556_RESET;
    Ak4556::Init(codec_reset_pin);

    // Reinit Audio for _both_ codecs...
    AudioHandle::Config cfg;
    cfg.blocksize  = 48;
    cfg.samplerate = SaiHandle::Config::SampleRate::SAI_48KHZ;
    cfg.postgain   = 0.5f;
    seed.audio_handle.Init(cfg, sai_handle[0], sai_handle[1]);
}

void DaisyPatch::InitControls()
{
    AdcChannelConfig cfg[CTRL_LAST];

    // Init ADC channels with Pins
    cfg[CTRL_1].InitSingle(PIN_CTRL_1);
    cfg[CTRL_2].InitSingle(PIN_CTRL_2);
    cfg[CTRL_3].InitSingle(PIN_CTRL_3);
    cfg[CTRL_4].InitSingle(PIN_CTRL_4);

    // Initialize ADC
    seed.adc.Init(cfg, CTRL_LAST);

    // Initialize AnalogControls, with flip set to true
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].Init(seed.adc.GetPtr(i), AudioCallbackRate(), true);
    }
}

void DaisyPatch::InitDisplay()
{
    OledDisplay<SSD130x4WireSpi128x64Driver>::Config display_config;

    display_config.driver_config.transport_config.pin_config.dc = PIN_OLED_DC;
    display_config.driver_config.transport_config.pin_config.reset
        = PIN_OLED_RESET;

    display.Init(display_config);
}

void DaisyPatch::InitMidi()
{
    MidiUartHandler::Config midi_config;
    midi.Init(midi_config);
}

void DaisyPatch::InitCvOutputs()
{
    //    dsy_dac_init(&seed.dac_handle, DSY_DAC_CHN_BOTH);
    //    dsy_dac_write(DSY_DAC_CHN1, 0);
    //    dsy_dac_write(DSY_DAC_CHN2, 0);
    DacHandle::Config cfg;
    cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
    cfg.buff_state = DacHandle::BufferState::ENABLED;
    cfg.mode       = DacHandle::Mode::POLLING;
    cfg.chn        = DacHandle::Channel::BOTH;
    seed.dac.Init(cfg);
    seed.dac.WriteValue(DacHandle::Channel::BOTH, 0);
}

void DaisyPatch::InitEncoder()
{
    encoder.Init(PIN_ENC_A, PIN_ENC_B, PIN_ENC_CLICK);
}

void DaisyPatch::InitGates()
{
    // Gate Output
    gate_output.pin  = PIN_GATE_OUT;
    gate_output.mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output);

    // Gate Inputs
    gate_input[GATE_IN_1].Init(PIN_GATE_IN_1);
    gate_input[GATE_IN_2].Init(PIN_GATE_IN_2);
}
