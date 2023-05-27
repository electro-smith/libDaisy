#include "daisy_legio.h"

using namespace daisy;

constexpr Pin PIN_SW_ENC          = seed::D1;
constexpr Pin PIN_TRIG_GATE       = seed::D18;
constexpr Pin PIN_TOGGLE3_RIGHT_A = seed::D30;
constexpr Pin PIN_TOGGLE3_RIGHT_B = seed::D29;
constexpr Pin PIN_TOGGLE3_LEFT_A  = seed::D17;
constexpr Pin PIN_TOGGLE3_LEFT_B  = seed::D15;
constexpr Pin PIN_ENC_A           = seed::D2;
constexpr Pin PIN_ENC_B           = seed::D3;

constexpr Pin PIN_LED_LEFT_R  = seed::D28;
constexpr Pin PIN_LED_LEFT_G  = seed::D6;
constexpr Pin PIN_LED_LEFT_B  = seed::D5;
constexpr Pin PIN_LED_RIGHT_R = seed::D26;
constexpr Pin PIN_LED_RIGHT_G = seed::D25;
constexpr Pin PIN_LED_RIGHT_B = seed::D24;

constexpr Pin PIN_ADC_PITCH0 = seed::D22;
constexpr Pin PIN_ADC_PITCH1 = seed::D23;
constexpr Pin PIN_ADC_CV0    = seed::D19;
constexpr Pin PIN_ADC_CV1    = seed::D21;

void DaisyLegio::Init(bool boost)
{
    // seed init
    seed.Configure();
    seed.Init(boost);
    seed.SetAudioBlockSize(48);
    float blockrate_ = seed.AudioSampleRate() / (float)seed.AudioBlockSize();

    Pin toggle_pina[] = {PIN_TOGGLE3_LEFT_A, PIN_TOGGLE3_RIGHT_A};
    Pin toggle_pinb[] = {PIN_TOGGLE3_LEFT_B, PIN_TOGGLE3_RIGHT_B};
    Pin ledr_pin[]    = {PIN_LED_LEFT_R, PIN_LED_RIGHT_R};
    Pin ledg_pin[]    = {PIN_LED_LEFT_G, PIN_LED_RIGHT_G};
    Pin ledb_pin[]    = {PIN_LED_LEFT_B, PIN_LED_RIGHT_B};
    Pin adc_pin[]     = {PIN_ADC_PITCH0, PIN_ADC_CV0, PIN_ADC_CV1};

    // push-button encoder
    encoder.Init(PIN_ENC_A, PIN_ENC_B, PIN_SW_ENC);

    // gate CV gate
    gate.Init(PIN_TRIG_GATE, false);

    // 3-position switches
    for(size_t i = 0; i < SW_LAST; i++)
    {
        sw[i].Init(toggle_pina[i], toggle_pinb[i]);
    }

    // ADC
    AdcChannelConfig adc_cfg[CONTROL_LAST];
    for(size_t i = 0; i < CONTROL_LAST; i++)
    {
        adc_cfg[i].InitSingle(adc_pin[i]);
    }
    seed.adc.Init(adc_cfg, CONTROL_LAST);

    for(size_t i = 0; i < CONTROL_LAST; i++)
    {
        controls[i].Init(seed.adc.GetPtr(i), blockrate_, true);
    }

    // RGB LEDs
    for(size_t i = 0; i < LED_LAST; i++)
    {
        leds[i].Init(ledr_pin[i], ledg_pin[i], ledb_pin[i], true);
    }
}

void DaisyLegio::SetHidUpdateRates()
{
    for(size_t i = 0; i < CONTROL_LAST; i++)
    {
        controls[i].SetSampleRate(AudioCallbackRate());
    }
}

void DaisyLegio::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyLegio::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyLegio::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyLegio::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyLegio::StopAudio()
{
    seed.StopAudio();
}

void DaisyLegio::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisyLegio::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

void DaisyLegio::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float DaisyLegio::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

float DaisyLegio::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisyLegio::StartAdc()
{
    seed.adc.Start();
}

void DaisyLegio::StopAdc()
{
    seed.adc.Stop();
}

void DaisyLegio::ProcessDigitalControls()
{
    encoder.Debounce();
}

void DaisyLegio::ProcessAnalogControls()
{
    for(size_t i = 0; i < CONTROL_LAST; i++)
    {
        controls[i].Process();
    }
}

bool DaisyLegio::Gate()
{
    return !gate.State();
}

void DaisyLegio::SetLed(size_t idx, float red, float green, float blue)
{
    leds[idx].Set(red, green, blue);
}

void DaisyLegio::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisyLegio::UpdateLeds()
{
    for(size_t i = 0; i < LED_LAST; i++)
    {
        leds[i].Update();
    }
}

float DaisyLegio::GetKnobValue(int idx)
{
    return controls[idx].Value();
}

void DaisyLegio::UpdateExample()
{
    encoder.Debounce();
    SetLed(LED_LEFT,
           sw[SW_LEFT].Read() / 2.0,
           controls[CONTROL_PITCH].Value(),
           controls[CONTROL_KNOB_TOP].Value());
    SetLed(LED_RIGHT,
           sw[SW_RIGHT].Read() / 2.0,
           controls[CONTROL_KNOB_BOTTOM].Value(),
           gate.State() ? 1.0 : 0.0);
}