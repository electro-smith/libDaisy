#include "daisy_pod.h"

#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE
#endif

#define SW_1_PORT seed_ports[29]
#define SW_1_PIN seed_pins[29]
#define SW_2_PORT seed_ports[28]
#define SW_2_PIN seed_pins[28]

#define ENC_A_PORT seed_ports[27]
#define ENC_A_PIN seed_pins[27]
#define ENC_B_PORT seed_ports[26]
#define ENC_B_PIN seed_pins[26]
#define ENC_CLICK_PORT seed_ports[1]
#define ENC_CLICK_PIN seed_pins[1]

#define LED_1_R_PORT seed_ports[21]
#define LED_1_R_PIN seed_pins[21]
#define LED_1_G_PORT seed_ports[20]
#define LED_1_G_PIN seed_pins[20]
#define LED_1_B_PORT seed_ports[19]
#define LED_1_B_PIN seed_pins[19]

#define LED_2_R_PORT seed_ports[0]
#define LED_2_R_PIN seed_pins[0]
#define LED_2_G_PORT seed_ports[25]
#define LED_2_G_PIN seed_pins[25]
#define LED_2_B_PORT seed_ports[24]
#define LED_2_B_PIN seed_pins[24]

using namespace daisy;

void DaisyPod::Init()
{
    // Set Some numbers up for accessors.
    sample_rate_ = SAMPLE_RATE; // TODO add configurable SR to libdaisy audio.
    block_size_ = 48; 
    callback_rate_ = (sample_rate_ / static_cast<float>(block_size_);
    // Initialize the hardware.
    daisy_seed_init(&seed);
    InitButtons();
    InitEncoder();
    InitLeds();
    InitKnobs();
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
    block_size_ = size;
    callback_rate_ = (sample_rate_ / static_cast<float>(block_size_);
}

float DaisyPod::AudioSampleRate() const
{
    return sample_rate_;
}

size_t DaisyPod::AudioBlockSize() const
{
    return block_size_;
}

float DaisyPod::AudioCallbackSize() const
{
    return callback_rate_;
}

void DaisyPod::StartAdc()
{
    dsy_adc_start();
}

float DaisyPod::UpdateKnobs()
{
    knob1.Process();
    knob2.Process();
}

float DaisyPod::GetKnobValue(Knob k)
{
    size_t idx;
    idx = k < LED_LAST ? static_cast<size_t>(k) : 0;
    return knobs[idx]->Value();
}

void DaisyPod::DebounceControls()
{
    encoder.Debounce();
    button1.Debounce();
    button2.Debounce();
}

void DaisyPod::SetLed(Led ld, float bright)
{
    // LEDs are inverted due to hardware.
    dsy_gpio_write(&leds[ld], bright > 0.0f ? 0 : 1);
}

void DaisyPod::ClearLeds()
{
    for(int i = 0; i < LED_LAST; i++)
    {
        // LEDs are inverted due to hardware.
        SetLed(static_cast<Led>(i), 1);
    }
}

void DaisyPod::UpdateLeds()
{
    // Does nothing on this platform at this time.
}

void DaisyPod::InitButtons()
{
    // button1
    dsy_gpio_pin button1_pin;
    button1_pin.pin  = SW_1_PIN;
    button1_pin.port = SW_1_PORT;
    button1.Init(button1_pin, callback_rate_);

    // button2
    dsy_gpio_pin button2_pin;
    button2_pin.pin  = SW_2_PIN;
    button2_pin.port = SW_2_PORT;
    button2.Init(button2_pin, callback_rate_);
    // ideal code:
    // in place until we fix the seed so that this is possible.
    //button2.Init(seed.ADC1, callback_rate_);
    //button2.Init(seed.pin(1), callback_rate_); 

    buttons[BUTTON_1] = &button1;
    buttons[BUTTON_2] = &button2;
}

void DaisyPod::InitEncoder()
{
    encoder.Init({ENC_A_PORT, ENC_A_PIN},
                 {ENC_B_PORT, ENC_B_PIN},
                 {ENC_CLICK_PORT, ENC_CLICK_PIN},
                 callback_rate_);
}

void DaisyPod::InitLeds()
{
    // LEDs are just going to be on/off for now.
    // TODO: Add PWM support

    dsy_gpio_port led_ports[LED_LAST] = {LED_1_R_PORT,
                                         LED_1_G_PORT,
                                         LED_1_B_PORT,
                                         LED_2_R_PORT,
                                         LED_2_G_PORT,
                                         LED_2_B_PORT};
    uint8_t       led_pins[LED_LAST]  = {LED_1_R_PIN,
                                  LED_1_G_PIN,
                                  LED_1_B_PIN,
                                  LED_2_R_PIN,
                                  LED_2_G_PIN,
                                  LED_2_B_PIN};

    for(uint8_t i = 0; i < LED_LAST; i++)
    {
        leds[i].pin.port = led_ports[i];
        leds[i].pin.pin  = led_pins[i];
        leds[i].mode     = DSY_GPIO_MODE_OUTPUT_PP;
        leds[i].pull     = DSY_GPIO_NOPULL;
        dsy_gpio_init(&leds[i]);
    }
}
void DaisyPod::InitKnobs()
{
    // Set order of ADCs based on CHANNEL NUMBER
    uint8_t channel_order[KNOB_LAST] = {DSY_ADC_PIN_CHN11, DSY_ADC_PIN_CHN10};
    // NUMBER OF CHANNELS
    seed.adc_handle.channels         = KNOB_LAST;
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
    for(int i = 0; i < KNOB_LAST; i++) {
        knobs[i]->Init(dsy_adc_get_rawptr(i), callback_rate_);
    }
}
