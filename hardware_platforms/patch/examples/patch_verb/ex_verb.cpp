#include "daisysp.h"
#include "daisy_patch.h"
using namespace daisy;
using namespace daisysp;
static daisy_patch patch;
static reverbsc verb;
static dcblock blk[2];
//static dsy_dcblock dcblock[2];
static float drylevel, send;

static void VerbCallback(float *in, float *out, size_t size)
{
    float dryL, dryR, wetL, wetR, sendL, sendR;
    for (size_t i = 0; i < size; i += 2)
    {
        //verb.set_feedback(0.15f + (dsy_adc_get_float(KNOB_1) * 0.85f));
        //verb.set_lpfreq(200.0f + (dsy_adc_get_float(KNOB_2) * 18000.0f));
        drylevel = dsy_adc_get_float(KNOB_3);
        send = dsy_adc_get_float(KNOB_4);
        dryL = in[i];
        dryR = in[i+1];
        sendL = dryL * send;
        sendR = dryR * send;
        verb.process(sendL, sendR, &wetL, &wetR);
        wetL = blk[0].process(wetL);
        wetR = blk[1].process(wetR);
        out[i] = (dryL * drylevel) + wetL;
        out[i + 1] = (dryR * drylevel) + wetR;
    }
}

int main(void)
{
    patch.init();
    verb.init(SAMPLE_RATE);
    verb.set_feedback(0.85f);
    verb.set_lpfreq(18000.0f);
    blk[0].init(SAMPLE_RATE);
    blk[1].init(SAMPLE_RATE);
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, VerbCallback);
    dsy_adc_start();
    dsy_audio_start(DSY_AUDIO_INTERNAL);
    while(1) {}
}
