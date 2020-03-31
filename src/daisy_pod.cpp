#include "daisy_pod.h"

#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE
#endif


#ifdef SEED_REV3

// Rev2 Pinout
// Compatible with Seed rev1 and rev2

#define SW_1_PIN 29
#define SW_2_PIN 28

#define ENC_A_PIN 27
#define ENC_B_PIN 26
#define ENC_CLICK_PIN 14

#define LED_1_R_PIN 21
#define LED_1_G_PIN 20
#define LED_1_B_PIN 19
#define LED_2_R_PIN 0
#define LED_2_G_PIN 25
#define LED_2_B_PIN 24

#else

// Rev1 Pinout
// Compatible with Seed rev1 and rev2

#define SW_1_PIN 29
#define SW_2_PIN 28

#define ENC_A_PIN 27
#define ENC_B_PIN 26
#define ENC_CLICK_PIN 1

#define LED_1_R_PIN 21
#define LED_1_G_PIN 20
#define LED_1_B_PIN 19
#define LED_2_R_PIN 0
#define LED_2_G_PIN 25
#define LED_2_B_PIN 24

#endif



using namespace daisy;

void DaisyPod::Init()
{
    // Set Some numbers up for accessors.
    sample_rate_   = SAMPLE_RATE; // TODO add configurable SR to libdaisy audio.
    block_size_    = 48;
    callback_rate_ = (sample_rate_ / static_cast<float>(block_size_));
    // Initialize the hardware.
    seed.Configure();
    seed.Init();
    dsy_tim_start();
    InitButtons();
    InitEncoder();
    InitLeds();
    InitKnobs();
    SetAudioBlockSize(block_size_);
}

void DaisyPod::DelayMs(size_t del)
{
    dsy_tim_delay_ms(del);
}

void DaisyPod::StartAudio(dsy_audio_callback cb)
{
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, cb);
    dsy_audio_start(DSY_AUDIO_INTERNAL);
}

void DaisyPod::ChangeAudioCallback(dsy_audio_callback cb)
{
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, cb);
}

void DaisyPod::SetAudioBlockSize(size_t size)
{
    block_size_    = size;
    callback_rate_ = (sample_rate_ / static_cast<float>(block_size_));
    dsy_audio_set_blocksize(DSY_AUDIO_INTERNAL, block_size_);
}

float DaisyPod::AudioSampleRate()
{
    return sample_rate_;
}

size_t DaisyPod::AudioBlockSize()
{
    return block_size_;
}

float DaisyPod::AudioCallbackRate()
{
    return callback_rate_;
}

void DaisyPod::StartAdc()
{
    dsy_adc_start();
}

void DaisyPod::UpdateAnalogControls()
{
    knob1.Process();
    knob2.Process();
}

float DaisyPod::GetKnobValue(Knob k)
{
    size_t idx;
    idx = k < KNOB_LAST ? k : KNOB_1;
    return knobs[idx]->Value();
}

void DaisyPod::DebounceControls()
{
    encoder.Debounce();
    button1.Debounce();
    button2.Debounce();
}

void DaisyPod::ClearLeds()
{
    // Using Color
    Color c;
    c.Init(Color::PresetColor::OFF);
    led1.SetColor(c);
    led2.SetColor(c);
    // Without
    // led1.Set(0.0f, 0.0f, 0.0f);
    // led2.Set(0.0f, 0.0f, 0.0f);
}

void DaisyPod::UpdateLeds()
{
    led1.Update();
    led2.Update();
}

void DaisyPod::InitButtons()
{
    // button1
    button1.Init(seed.GetPin(SW_1_PIN), callback_rate_);
    // button2
    button2.Init(seed.GetPin(SW_2_PIN), callback_rate_);

    buttons[BUTTON_1] = &button1;
    buttons[BUTTON_2] = &button2;
}

void DaisyPod::InitEncoder()
{
    dsy_gpio_pin a, b, click;
    a = seed.GetPin(ENC_A_PIN);
    b = seed.GetPin(ENC_B_PIN);
    click = seed.GetPin(ENC_CLICK_PIN);
    encoder.Init(a, b, click, callback_rate_);
}

void DaisyPod::InitLeds()
{
    // LEDs are just going to be on/off for now.
    // TODO: Add PWM support
    dsy_gpio_pin rpin, gpin, bpin;

    rpin = seed.GetPin(LED_1_R_PIN);
    gpin = seed.GetPin(LED_1_G_PIN);
    bpin = seed.GetPin(LED_1_B_PIN);
    led1.Init(rpin, gpin, bpin, true);

    rpin = seed.GetPin(LED_2_R_PIN);
    gpin = seed.GetPin(LED_2_G_PIN);
    bpin = seed.GetPin(LED_2_B_PIN);
    led2.Init(rpin, gpin, bpin, true);

    ClearLeds();
    UpdateLeds();
}
void DaisyPod::InitKnobs()
{
    // Set order of ADCs based on CHANNEL NUMBER
    uint8_t channel_order[KNOB_LAST] = {DSY_ADC_PIN_CHN4, DSY_ADC_PIN_CHN10};
    // NUMBER OF CHANNELS
    seed.adc_handle.channels = KNOB_LAST;
    // Fill the ADCs active channel array.
    for(uint8_t i = 0; i < KNOB_LAST; i++)
    {
        seed.adc_handle.active_channels[i] = channel_order[i];
    }
    // Set Oversampling to 32x
    seed.adc_handle.oversampling = DSY_ADC_OVS_32;
    // Init ADC
    dsy_adc_init(&seed.adc_handle);
    // Make an array of pointers to the knobs.
    knobs[KNOB_1] = &knob1;
    knobs[KNOB_2] = &knob2;
    for(int i = 0; i < KNOB_LAST; i++)
    {
        knobs[i]->Init(dsy_adc_get_rawptr(i), callback_rate_);
    }
}
