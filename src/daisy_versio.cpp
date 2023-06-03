#include "daisy_versio.h"

using namespace daisy;

constexpr Pin PIN_TRIG_IN    = seed::D24;
constexpr Pin PIN_SW         = seed::D30;
constexpr Pin PIN_TOGGLE3_0A = seed::D6;
constexpr Pin PIN_TOGGLE3_0B = seed::D5;
constexpr Pin PIN_TOGGLE3_1A = seed::D1;
constexpr Pin PIN_TOGGLE3_1B = seed::D0;

constexpr Pin PIN_LED0_R = seed::D10;
constexpr Pin PIN_LED0_G = seed::D3;
constexpr Pin PIN_LED0_B = seed::D4;
constexpr Pin PIN_LED1_R = seed::D12;
constexpr Pin PIN_LED1_G = seed::D13;
constexpr Pin PIN_LED1_B = seed::D11;
constexpr Pin PIN_LED2_R = seed::D25;
constexpr Pin PIN_LED2_G = seed::D26;
constexpr Pin PIN_LED2_B = seed::D14;
constexpr Pin PIN_LED3_R = seed::D29;
constexpr Pin PIN_LED3_G = seed::D27;
constexpr Pin PIN_LED3_B = seed::D15;

constexpr Pin PIN_ADC_CV0 = seed::D21;
constexpr Pin PIN_ADC_CV1 = seed::D22;
constexpr Pin PIN_ADC_CV2 = seed::D28;
constexpr Pin PIN_ADC_CV3 = seed::D23;
constexpr Pin PIN_ADC_CV4 = seed::D16;
constexpr Pin PIN_ADC_CV5 = seed::D17;
constexpr Pin PIN_ADC_CV6 = seed::D19;


void DaisyVersio::Init(bool boost)
{
    // seed init
    seed.Configure();
    seed.Init(boost);
    seed.SetAudioBlockSize(48);
    float blockrate_ = seed.AudioSampleRate() / (float)seed.AudioBlockSize();

    // pin numbers
    constexpr Pin toggle_pina[] = {PIN_TOGGLE3_0A, PIN_TOGGLE3_1A};
    constexpr Pin toggle_pinb[] = {PIN_TOGGLE3_0B, PIN_TOGGLE3_1B};
    constexpr Pin ledr_pin[] = {PIN_LED0_R, PIN_LED1_R, PIN_LED2_R, PIN_LED3_R};
    constexpr Pin ledg_pin[] = {PIN_LED0_G, PIN_LED1_G, PIN_LED2_G, PIN_LED3_G};
    constexpr Pin ledb_pin[] = {PIN_LED0_B, PIN_LED1_B, PIN_LED2_B, PIN_LED3_B};
    constexpr Pin adc_pin[]  = {PIN_ADC_CV0,
                               PIN_ADC_CV1,
                               PIN_ADC_CV2,
                               PIN_ADC_CV3,
                               PIN_ADC_CV4,
                               PIN_ADC_CV5,
                               PIN_ADC_CV6};

    // gate in and momentary switch
    tap.Init(PIN_SW);
    gate.Init(PIN_TRIG_IN);


    // 3-position switches
    for(size_t i = 0; i < SW_LAST; i++)
    {
        sw[i].Init(toggle_pina[i], toggle_pinb[i]);
    }

    // ADC
    AdcChannelConfig adc_cfg[KNOB_LAST];
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        adc_cfg[i].InitSingle(adc_pin[i]);
    }
    seed.adc.Init(adc_cfg, 7);

    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knobs[i].Init(seed.adc.GetPtr(i), blockrate_, true);
    }

    // RGB LEDs
    for(size_t i = 0; i < LED_LAST; i++)
    {
        leds[i].Init(ledr_pin[i], ledg_pin[i], ledb_pin[i], true);
    }
}

void DaisyVersio::SetHidUpdateRates()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knobs[i].SetSampleRate(AudioCallbackRate());
    }
}


void DaisyVersio::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyVersio::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyVersio::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyVersio::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyVersio::StopAudio()
{
    seed.StopAudio();
}

void DaisyVersio::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisyVersio::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

void DaisyVersio::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float DaisyVersio::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

float DaisyVersio::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisyVersio::StartAdc()
{
    seed.adc.Start();
}

void DaisyVersio::StopAdc()
{
    seed.adc.Stop();
}

void DaisyVersio::ProcessAnalogControls()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knobs[i].Process();
    }
}

bool DaisyVersio::SwitchPressed()
{
    return tap.Pressed();
}

bool DaisyVersio::Gate()
{
    return !gate.State();
}

void DaisyVersio::SetLed(size_t idx, float red, float green, float blue)
{
    leds[idx].Set(red, green, blue);
}

void DaisyVersio::DelayMs(size_t del)
{
    seed.DelayMs(del);
}

void DaisyVersio::UpdateLeds()
{
    for(size_t i = 0; i < LED_LAST; i++)
    {
        leds[i].Update();
    }
}

float DaisyVersio::GetKnobValue(int idx)
{
    return knobs[idx].Value();
}

void DaisyVersio::UpdateExample()
{
    tap.Debounce();
    for(size_t i = 0; i < LED_LAST - 1; i++)
        SetLed(i, 0, 0, 0);

    SetLed(sw[0].Read(), knobs[0].Value(), knobs[1].Value(), knobs[2].Value());
    SetLed(sw[1].Read(), knobs[3].Value(), knobs[4].Value(), knobs[5].Value());

    SetLed(3, Gate(), SwitchPressed(), knobs[6].Value());
}
