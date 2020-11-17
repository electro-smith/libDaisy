#include "daisy_versio.h"

using namespace daisy;

#define PIN_TRIG_IN 24
#define PIN_SW 30
#define PIN_TOGGLE3_0A 6
#define PIN_TOGGLE3_0B 5
#define PIN_TOGGLE3_1A 1
#define PIN_TOGGLE3_1B 0

#define PIN_LED0_R 10
#define PIN_LED0_G 3
#define PIN_LED0_B 4
#define PIN_LED1_R 12
#define PIN_LED1_G 13
#define PIN_LED1_B 11
#define PIN_LED2_R 25
#define PIN_LED2_G 26
#define PIN_LED2_B 14
#define PIN_LED3_R 29
#define PIN_LED3_G 27
#define PIN_LED3_B 15

#define PIN_ADC_CV0 21
#define PIN_ADC_CV1 22
#define PIN_ADC_CV2 28
#define PIN_ADC_CV3 23
#define PIN_ADC_CV4 16
#define PIN_ADC_CV5 17
#define PIN_ADC_CV6 19


void DaisyVersio::Init()
{
    // seed init
    seed.Configure();
    seed.Init();
    samplerate_ = seed.AudioSampleRate();
    blocksize_  = 48;
    seed.SetAudioBlockSize(blocksize_);
    blockrate_ = samplerate_ / (float)blocksize_;

    // pin numbers
    uint8_t toggle_pina[] = {PIN_TOGGLE3_0A, PIN_TOGGLE3_1A};
    uint8_t toggle_pinb[] = {PIN_TOGGLE3_0B, PIN_TOGGLE3_1B};
    uint8_t ledr_pin[]    = {PIN_LED0_R, PIN_LED1_R, PIN_LED2_R, PIN_LED3_R};
    uint8_t ledg_pin[]    = {PIN_LED0_G, PIN_LED1_G, PIN_LED2_G, PIN_LED3_G};
    uint8_t ledb_pin[]    = {PIN_LED0_B, PIN_LED1_B, PIN_LED2_B, PIN_LED3_B};
    uint8_t adc_pin[]     = {PIN_ADC_CV0,
                         PIN_ADC_CV1,
                         PIN_ADC_CV2,
                         PIN_ADC_CV3,
                         PIN_ADC_CV4,
                         PIN_ADC_CV5,
                         PIN_ADC_CV6};

    // gate in and momentary switch
    tap_.Init(seed.GetPin(PIN_SW), blockrate_);
    dsy_gpio_pin gate_gpio_ = seed.GetPin(PIN_TRIG_IN);
    gate_.Init(&gate_gpio_);


    // 3-position switches
    for(size_t i = 0; i < SW_LAST; i++)
    {
        sw_[i].Init(seed.GetPin(toggle_pina[i]), seed.GetPin(toggle_pinb[i]));
    }

    // ADC
    AdcChannelConfig adc_cfg[KNOB_LAST];
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        adc_cfg[i].InitSingle(seed.GetPin(adc_pin[i]));
    }
    seed.adc.Init(adc_cfg, 7);

    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knobs_[i].Init(seed.adc.GetPtr(i), blockrate_, true);
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

void DaisyVersio::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyVersio::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyVersio::DelayMs(size_t del)
{
    dsy_system_delay(del);
}

void DaisyVersio::UpdateLeds()
{
    for(size_t i = 0; i < LED_LAST; i++)
    {
        leds[i].Update();
    }
}

void DaisyVersio::ProcessAnalogControls()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knobs_[i].Process();
    }
}

float DaisyVersio::GetKnobValue(int idx)
{
    return knobs_[idx].Value();
}

void DaisyVersio::UpdateExample()
{
    tap_.Debounce();
    for(size_t i = 0; i < LED_LAST - 1; i++)
        SetLed(i, 0, 0, 0);

    SetLed(
        sw_[0].Read(), knobs_[0].Value(), knobs_[1].Value(), knobs_[2].Value());
    SetLed(
        sw_[1].Read(), knobs_[3].Value(), knobs_[4].Value(), knobs_[5].Value());

    SetLed(3, Gate(), SwitchPressed(), knobs_[6].Value());
}
