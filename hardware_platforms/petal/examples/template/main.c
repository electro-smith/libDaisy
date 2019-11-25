#include "dsy_petal_bsp.h"
//#include "daisysp.h" // Uncomment this if you want to use the DSP library.

// Declare a local daisy_petal for hardware access
daisy_petal hw;

// This runs at a fixed rate, to prepare audio samples
void callback(float *in, float *out, size_t size)
{
    // Audio is interleaved stereo by default
    for (size_t i = 0; i < size; i+=2)
    {
        out[i] = in[i]; // left
        out[i+1] = in[i+1]; // right
    }
}

int main(void)
{
    daisy_petal_init(&hw);
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, callback);
    // Initialize Stuff Here
    while(1) 
    {
        // Do Stuff Infinitely Here
    }
}
