#include "daisy_petal.h"

using namespace daisy;

#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE
#endif

// Hardware related defines.
// Switches
#define SW_1_PIN 9
#define SW_2_PIN 10
#define SW_3_PIN 11
#define SW_4_PIN 14
#define SW_5_PIN 0
#define SW_6_PIN 27
#define SW_7_PIN 8

// Encoder
#define ENC_A_PIN 29
#define ENC_B_PIN 28
#define ENC_CLICK_PIN 15

// Extra Peripherals
#define LED_DRIVER_I2C i2c1_handle

enum LedOrder
{
    LED_RING_1_R,
    LED_RING_1_G,
    LED_RING_1_B,
    LED_RING_5_R,
    LED_RING_5_G,
    LED_RING_5_B,
    LED_RING_2_R,
    LED_RING_2_G,
    LED_RING_2_B,
    LED_RING_6_R,
    LED_RING_6_G,
    LED_RING_6_B,
    LED_RING_3_R,
    LED_RING_3_G,
    LED_RING_3_B,
    LED_FS_1,
    LED_RING_4_R,
    LED_RING_4_G,
    LED_RING_4_B,
    LED_RING_7_R,
    LED_RING_7_G,
    LED_RING_7_B,
    LED_RING_8_R,
    LED_RING_8_G,
    LED_RING_8_B,
    LED_FS_2,
    LED_FS_3,
    LED_FS_4,
    LED_FAKE1,
    LED_FAKE2,
    LED_FAKE3,
    LED_FAKE4,
    LED_LAST,
};


void DaisyPetal::Init()
{
    // Set Some numbers up for accessors.
    sample_rate_   = SAMPLE_RATE; // TODO add configurable SR to libdaisy audio.
    block_size_    = 48;
    callback_rate_ = (sample_rate_ / static_cast<float>(block_size_));
    // Initialize the hardware.
    seed.Configure();
    seed.Init();
    dsy_tim_start();
    InitSwitches();
    InitEncoder();
    InitLeds();
    InitAnalogControls();
    SetAudioBlockSize(block_size_);
    seed.usb_handle.Init(UsbHandle::FS_BOTH);
}

void DaisyPetal::DelayMs(size_t del)
{
    dsy_tim_delay_ms(del);
}

void DaisyPetal::StartAudio(dsy_audio_callback cb)
{
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, cb);
    dsy_audio_start(DSY_AUDIO_INTERNAL);
}

void DaisyPetal::ChangeAudioCallback(dsy_audio_callback cb)
{
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, cb);
}

void DaisyPetal::SetAudioBlockSize(size_t size)
{
    block_size_    = size;
    callback_rate_ = (sample_rate_ / static_cast<float>(block_size_));
    dsy_audio_set_blocksize(DSY_AUDIO_INTERNAL, block_size_);
}

float DaisyPetal::AudioSampleRate()
{
    return sample_rate_;
}

size_t DaisyPetal::AudioBlockSize()
{
    return block_size_;
}

float DaisyPetal::AudioCallbackRate()
{
    return callback_rate_;
}

void DaisyPetal::StartAdc()
{
    dsy_adc_start();
}

void DaisyPetal::UpdateAnalogControls()
{
    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knob[i].Process();
    }
    expression.Process();
}

float DaisyPetal::GetKnobValue(Knob k)
{
    size_t idx;
    idx = k < KNOB_LAST ? k : KNOB_1;
    return knob[idx].Value();
}

void DaisyPetal::DebounceControls()
{
    encoder.Debounce();
    for(size_t i = 0; i < SW_LAST; i++)
    {
        switches[i].Debounce();
    }
}


void DaisyPetal::ClearLeds()
{
    // Using Color
    Color c;
    c.Init(Color::PresetColor::OFF);
    for(size_t i = 0; i < RING_LED_LAST; i++)
    {
        ring_led[i].SetColor(c);
    }
}

void DaisyPetal::UpdateLeds()
{
    // TODO:
    // Get the LED values into the LED Driver...
    // Still have to call this once per driver -- need to update.
    dsy_led_driver_update();
    dsy_led_driver_update();
}

void DaisyPetal::SetRingLed(RingLed idx, float r, float g, float b)
{
    uint8_t r_addr[RING_LED_LAST] = {LED_RING_1_R,
                                     LED_RING_2_R,
                                     LED_RING_3_R,
                                     LED_RING_4_R,
                                     LED_RING_5_R,
                                     LED_RING_6_R,
                                     LED_RING_7_R,
                                     LED_RING_8_R};
    uint8_t g_addr[RING_LED_LAST] = {LED_RING_1_G,
                                     LED_RING_2_G,
                                     LED_RING_3_G,
                                     LED_RING_4_G,
                                     LED_RING_5_G,
                                     LED_RING_6_G,
                                     LED_RING_7_G,
                                     LED_RING_8_G};
    uint8_t b_addr[RING_LED_LAST] = {LED_RING_1_B,
                                     LED_RING_2_B,
                                     LED_RING_3_B,
                                     LED_RING_4_B,
                                     LED_RING_5_B,
                                     LED_RING_6_B,
                                     LED_RING_7_B,
                                     LED_RING_8_B};

    dsy_led_driver_set_led(r_addr[idx], cube(r));
    dsy_led_driver_set_led(g_addr[idx], cube(g));
    dsy_led_driver_set_led(b_addr[idx], cube(b));
}
void DaisyPetal::SetFootswitchLed(FootswitchLed idx, float bright)
{
    uint8_t fs_addr[FOOTSWITCH_LED_LAST] = { LED_FS_1, LED_FS_2, LED_FS_3, LED_FS_4};
    dsy_led_driver_set_led(fs_addr[idx], cube(bright));
}

void DaisyPetal::InitSwitches()
{
    //    // button1
    //    button1.Init(seed.GetPin(SW_1_PIN), callback_rate_);
    //    // button2
    //    button2.Init(seed.GetPin(SW_2_PIN), callback_rate_);
    //
    //    buttons[BUTTON_1] = &button1;
    //    buttons[BUTTON_2] = &button2;
    uint8_t pin_numbers[SW_LAST] = {
        SW_1_PIN,
        SW_2_PIN,
        SW_3_PIN,
        SW_4_PIN,
        SW_5_PIN,
        SW_6_PIN,
        SW_7_PIN,
    };

    for(size_t i = 0; i < SW_LAST; i++)
    {
        switches[i].Init(seed.GetPin(pin_numbers[i]), callback_rate_);
    }
}

void DaisyPetal::InitEncoder()
{
    dsy_gpio_pin a, b, click;
    a     = seed.GetPin(ENC_A_PIN);
    b     = seed.GetPin(ENC_B_PIN);
    click = seed.GetPin(ENC_CLICK_PIN);
    encoder.Init(a, b, click, callback_rate_);
}

void DaisyPetal::InitLeds()
{
    // LEDs are on the LED Driver.

    // Need to figure out how we want to handle that.
    uint8_t addr[2] = {0x00, 0x01};
    dsy_led_driver_init(&seed.LED_DRIVER_I2C, addr, 2);
    ClearLeds();
    UpdateLeds();
}
void DaisyPetal::InitAnalogControls()
{
    // +1 to the arrays and iterator to account for expression
    // Expression will be at position KNOB_LAST (aka one after the
    //     end of the knob.

    // Set order of ADCs based on CHANNEL NUMBER
    uint8_t channel_order[KNOB_LAST + 1] = {DSY_ADC_PIN_CHN15,
                                            DSY_ADC_PIN_CHN5,
                                            DSY_ADC_PIN_CHN7,
                                            DSY_ADC_PIN_CHN3,
                                            DSY_ADC_PIN_CHN11,
                                            DSY_ADC_PIN_CHN4,
                                            DSY_ADC_PIN_CHN10};
    // NUMBER OF CHANNELS
    seed.adc_handle.channels = KNOB_LAST + 1;
    // Fill the ADCs active channel array.
    for(uint8_t i = 0; i < KNOB_LAST + 1; i++)
    {
        seed.adc_handle.active_channels[i] = channel_order[i];
    }
    // Set Oversampling to 32x
    seed.adc_handle.oversampling = DSY_ADC_OVS_32;
    // Init ADC
    dsy_adc_init(&seed.adc_handle);

    // Make an array of pointers to the knob.
    for(int i = 0; i < KNOB_LAST; i++)
    {
        knob[i].Init(dsy_adc_get_rawptr(i), callback_rate_);
    }
    expression.Init(dsy_adc_get_rawptr(KNOB_LAST), callback_rate_);
}
