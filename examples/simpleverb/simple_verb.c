#include "libdaisy.h"
#include "daisysp.h"
#include "dsy_patch_bsp.h"

static dsy_reverbsc_t verb;
static float drylevel, send;

static void VerbCallback(float *in, float *out, size_t size)
{
    float dryL, dryR, wetL, wetR, sendL, sendR;
    for (size_t i = 0; i < size; i += 2)
    {
        verb.feedback = 0.15f + (dsy_adc_get_float(DSY_PATCH_KNOB_1) * 0.85f);
        verb.lpfreq = 200.0f + (dsy_adc_get_float(DSY_PATCH_KNOB_2) * 18000.0f);
        drylevel = dsy_adc_get_float(DSY_PATCH_KNOB_3);
        send = dsy_adc_get_float(DSY_PATCH_KNOB_4);
        dryL = in[i];
        dryR = in[i+1];
        sendL = dryL * send;
        sendR = dryR * send;
        dsy_reverbsc_process(&verb, &sendL, &sendR, &wetL, &wetR);
        out[i] = (dryL * drylevel) + wetL;
        out[i + 1] = (dryR * drylevel) + wetR;
    }
}

int main(void)
{
    uint8_t board = DSY_SYS_BOARD_DAISY_SEED;
    dsy_system_init(board);
    dsy_audio_init(board, DSY_AUDIO_INTERNAL, DSY_AUDIO_DEVICE_WM8731);
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, VerbCallback);
    dsy_reverbsc_init(&verb, DSY_AUDIO_SAMPLE_RATE);
    dsy_adc_init(board);
    dsy_adc_start();
    dsy_audio_start(DSY_AUDIO_INTERNAL);
    while(1) {}
}
