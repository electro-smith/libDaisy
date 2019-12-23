// HID Tests:
// Reworking all hid_* modules to be C++ for better user-interface.
//
// I'll be using the daisy pod to test most of the hardware,
// but won't be using the Pod BSP. Just to make sure the
// initialization for each component is nice and clean.
//
// Once everything's ironed out. I'll back annotate the
// init changes, etc. to the bsp files.
#include <string.h>
#include "daisy_seed.h"
#include "daisysp.h"
#include <math.h>

using namespace daisy;

static void init_led();

// Functional Modules
daisy_handle hw;
dsy_gpio led_blue;
// Module in progress.
Switch sw_1;

enum class LedState
{
    ON  = 0x00,
    OFF = 0x01,
};

void AudioCallback(float *in, float *out, size_t size)
{
    // Ticks at FS/size (default 2kHz)
    sw_1.Debounce();
	if(sw_1.RisingEdge())
        dsy_gpio_write(&led_blue, static_cast<uint8_t>(LedState::ON));
    if(sw_1.TimeHeldMs() > 1000 || sw_1.FallingEdge())
        dsy_gpio_write(&led_blue, static_cast<uint8_t>(LedState::OFF));

    for(size_t i = 0; i < size; i += 2)
    {
        // Ticks at FS
        out[i]     = in[i];
        out[i + 1] = in[i + 1];
    }
}


int main(void)
{
    // Initialize Hardware
    daisy_seed_init(&hw);
    dsy_tim_start();
    init_led();
    // Testing New Switch
    sw_1.Init({seed_ports[28], seed_pins[28]},
              DSY_AUDIO_SAMPLE_RATE / 24,
              Switch::TYPE_MOMENTARY,
              Switch::POLARITY_NORMAL,
              Switch::PULL_UP);
    // Audio will get converted LAST
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, AudioCallback);
    dsy_audio_start(DSY_AUDIO_INTERNAL);

    while(1) {}
}

void init_led()
{
    // Still old style so as not to break
    // pure-c modules..
    led_blue.pin.port = seed_ports[19];
    led_blue.pin.pin  = seed_pins[19];
    led_blue.mode     = DSY_GPIO_MODE_OUTPUT_PP;
    led_blue.pull     = DSY_GPIO_NOPULL;
    dsy_gpio_init(&led_blue);
}
