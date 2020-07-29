#include "daisy_patch.h"

using namespace daisy;

// Hardware Definitions
#define PIN_ENC_CLICK 0
#define PIN_ENC_B 11
#define PIN_ENC_A 12
#define PIN_OLED_DC 9
#define PIN_OLED_RESET 30
#define PIN_MIDI_OUT 13
#define PIN_MIDI_IN 14
#define PIN_GATE_OUT 17
#define PIN_GATE_IN_1 20
#define PIN_GATE_IN_2 19
#define PIN_SAI_SCK_A 28
#define PIN_SAI2_FS_A 27
#define PIN_SAI2_SD_A 26
#define PIN_SAI2_SD_B 25
#define PIN_SAI2_MCLK 24

#define PIN_AK4556_RESET 29

#define PIN_CTRL_1 15
#define PIN_CTRL_2 16
#define PIN_CTRL_3 21
#define PIN_CTRL_4 18

const float kAudioSampleRate = DSY_AUDIO_SAMPLE_RATE;

void DaisyPatch::Init()
{
    // Configure Seed first
    seed.Configure();
    block_size_ = 48;
    InitAudio();
    seed.Init();
    InitDisplay();
    InitCvOutputs();
    InitEncoder();
    InitGates();
    InitDisplay();
    InitMidi();
    InitControls();
    // Reset AK4556
    dsy_gpio_write(&ak4556_reset_pin_, 0);
    DelayMs(10);
    dsy_gpio_write(&ak4556_reset_pin_, 1);
    // Set Screen update vars
    screen_update_period_ = 17; // roughly 60Hz
    screen_update_last_   = dsy_system_getnow();
}

void DaisyPatch::DelayMs(size_t del)
{
    dsy_system_delay(del);
}
void DaisyPatch::SetAudioBlockSize(size_t size)
{
    block_size_ = size;
    dsy_audio_set_blocksize(DSY_AUDIO_INTERNAL, block_size_);
    dsy_audio_set_blocksize(DSY_AUDIO_EXTERNAL, block_size_);
}

void DaisyPatch::StartAudio(dsy_audio_mc_callback cb)
{
    dsy_audio_set_mc_callback(cb);
    dsy_audio_start(DSY_AUDIO_INTERNAL);
    dsy_audio_start(DSY_AUDIO_EXTERNAL);
}

void DaisyPatch::ChangeAudioCallback(dsy_audio_mc_callback cb)
{
    dsy_audio_set_mc_callback(cb);
}

void DaisyPatch::StartAdc()
{
    seed.adc.Start();
}
float DaisyPatch::AudioSampleRate()
{
    return kAudioSampleRate;
}
size_t DaisyPatch::AudioBlockSize()
{
    return block_size_;
}
float DaisyPatch::AudioCallbackRate()
{
    return kAudioSampleRate / block_size_;
}
void DaisyPatch::UpdateAnalogControls()
{
    for(size_t i = 0; i < CTRL_LAST; i++)
    {
        controls[i].Process();
    }
}
float DaisyPatch::GetCtrlValue(Ctrl k)
{
    return (controls[k].Value());
}

void DaisyPatch::DebounceControls()
{
    encoder.Debounce();
}

// This will render the display with the controls as vertical bars
void DaisyPatch::DisplayControls(bool invert)
{
    bool on, off;
    on  = invert ? false : true;
    off = invert ? true : false;
    if(dsy_system_getnow() - screen_update_last_ > screen_update_period_)
    {
        // Graph Knobs
        size_t barwidth, barspacing;
        size_t curx, cury;
        barwidth   = 15;
        barspacing = 20;
        display.Fill(off);
        // Bars for all four knobs.
        for(size_t i = 0; i < DaisyPatch::CTRL_LAST; i++)
        {
            float  v;
            size_t dest;
            curx = (barspacing * i + 1) + (barwidth * i);
            cury = SSD1309_HEIGHT;
            v    = GetCtrlValue(static_cast<DaisyPatch::Ctrl>(i));
            dest = (v * SSD1309_HEIGHT);
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
    seed.sai_handle.init                   = DSY_AUDIO_INIT_BOTH;
    seed.sai_handle.device[DSY_SAI_2]      = DSY_AUDIO_DEVICE_AK4556;
    seed.sai_handle.samplerate[DSY_SAI_2]  = DSY_AUDIO_SAMPLERATE_48K;
    seed.sai_handle.bitdepth[DSY_SAI_2]    = DSY_AUDIO_BITDEPTH_24;
    seed.sai_handle.a_direction[DSY_SAI_2] = DSY_AUDIO_TX;
    seed.sai_handle.b_direction[DSY_SAI_2] = DSY_AUDIO_RX;
    seed.sai_handle.sync_config[DSY_SAI_2] = DSY_AUDIO_SYNC_MASTER;

    ak4556_reset_pin_.pin  = seed.GetPin(PIN_AK4556_RESET);
    ak4556_reset_pin_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    ak4556_reset_pin_.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&ak4556_reset_pin_);

    dsy_audio_set_blocksize(DSY_AUDIO_INTERNAL, block_size_);
    dsy_audio_set_blocksize(DSY_AUDIO_EXTERNAL, block_size_);
}
void DaisyPatch::InitControls()
{
    AdcChannelConfig cfg[CTRL_LAST];

    // Init ADC channels with Pins
    cfg[CTRL_1].InitSingle(seed.GetPin(PIN_CTRL_1));
    cfg[CTRL_2].InitSingle(seed.GetPin(PIN_CTRL_2));
    cfg[CTRL_3].InitSingle(seed.GetPin(PIN_CTRL_3));
    cfg[CTRL_4].InitSingle(seed.GetPin(PIN_CTRL_4));

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
    dsy_gpio_pin pincfg[OledDisplay::NUM_PINS];
    pincfg[OledDisplay::DATA_COMMAND] = seed.GetPin(PIN_OLED_DC);
    pincfg[OledDisplay::RESET]        = seed.GetPin(PIN_OLED_RESET);
    display.Init(pincfg);
}

void DaisyPatch::InitMidi()
{
    midi.Init(MidiHandler::MidiInputMode::INPUT_MODE_UART1,
              MidiHandler::MidiOutputMode::OUTPUT_MODE_UART1);
}

void DaisyPatch::InitCvOutputs()
{
    dsy_dac_init(&seed.dac_handle, DSY_DAC_CHN_BOTH);
    dsy_dac_write(DSY_DAC_CHN1, 0);
    dsy_dac_write(DSY_DAC_CHN2, 0);
}

void DaisyPatch::InitEncoder()
{
    encoder.Init(seed.GetPin(PIN_ENC_A),
                 seed.GetPin(PIN_ENC_B),
                 seed.GetPin(PIN_ENC_CLICK),
                 AudioCallbackRate());
}

void DaisyPatch::InitGates()
{
    // Gate Output
    gate_output.pin  = seed.GetPin(PIN_GATE_OUT);
    gate_output.mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_output.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&gate_output);

    // Gate Inputs
    dsy_gpio_pin pin;
    pin = seed.GetPin(PIN_GATE_IN_1);
    gate_input[GATE_IN_1].Init(&pin);
    pin = seed.GetPin(PIN_GATE_IN_2);
    gate_input[GATE_IN_2].Init(&pin);
}
