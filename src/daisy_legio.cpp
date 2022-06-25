#include "daisy_legio.h"

using namespace daisy;

#define PIN_SW_ENC 1
#define PIN_TRIG_GATE 18
#define PIN_TOGGLE3_RIGHT_A 30
#define PIN_TOGGLE3_RIGHT_B 29
#define PIN_TOGGLE3_LEFT_A 17
#define PIN_TOGGLE3_LEFT_B 15
#define PIN_ENC_A 2
#define PIN_ENC_B 3

#define PIN_LED_LEFT_R 28
#define PIN_LED_LEFT_G 6
#define PIN_LED_LEFT_B 5
#define PIN_LED_RIGHT_R 26
#define PIN_LED_RIGHT_G 25
#define PIN_LED_RIGHT_B 24

#define PIN_ADC_PITCH0 22
#define PIN_ADC_PITCH1 23
#define PIN_ADC_CV0 19
#define PIN_ADC_CV1 21

void DaisyLegio::Init(bool boost)
{
    // seed init
    seed.Configure();
    seed.Init(boost);
    seed.SetAudioBlockSize(48);
    float blockrate_ = seed.AudioSampleRate() / (float)seed.AudioBlockSize();

    uint8_t toggle_pina[] = {PIN_TOGGLE3_LEFT_A, PIN_TOGGLE3_RIGHT_A};
    uint8_t toggle_pinb[] = {PIN_TOGGLE3_LEFT_B, PIN_TOGGLE3_RIGHT_B};
    uint8_t ledr_pin[]    = {PIN_LED_LEFT_R, PIN_LED_RIGHT_R};
    uint8_t ledg_pin[]    = {PIN_LED_LEFT_G, PIN_LED_RIGHT_G};
    uint8_t ledb_pin[]    = {PIN_LED_LEFT_B, PIN_LED_RIGHT_B};
    uint8_t adc_pin[]     = {PIN_ADC_PITCH0, PIN_ADC_CV0, PIN_ADC_CV1};

    // push-button encoder
    encoder.Init(seed.GetPin(PIN_ENC_A),
                 seed.GetPin(PIN_ENC_B),
                 seed.GetPin(PIN_SW_ENC));

    // gate CV gate
    dsy_gpio_pin gate_gpio = seed.GetPin(PIN_TRIG_GATE);
    gate.Init(&gate_gpio, false);

    // 3-position switches
    for(size_t i = 0; i < SW_LAST; i++)
    {
        sw[i].Init(seed.GetPin(toggle_pina[i]), seed.GetPin(toggle_pinb[i]));
    }

    // ADC
    AdcChannelConfig adc_cfg[CONTROL_LAST];
    for(size_t i = 0; i < CONTROL_LAST; i++)
    {
        adc_cfg[i].InitSingle(seed.GetPin(adc_pin[i]));
    }
    seed.adc.Init(adc_cfg, CONTROL_LAST);

    for(size_t i = 0; i < CONTROL_LAST; i++)
    {
        controls[i].Init(seed.adc.GetPtr(i), blockrate_, true);
    }

    // RGB LEDs
    for(size_t i = 0; i < LED_LAST; i++)
    {
        dsy_gpio_pin r = seed.GetPin(ledr_pin[i]);
        dsy_gpio_pin g = seed.GetPin(ledg_pin[i]);
        dsy_gpio_pin b = seed.GetPin(ledb_pin[i]);
        leds[i].Init(r, g, b, true);
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