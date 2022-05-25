#include "hid/ctrl.h"
#include <math.h>
// Temporary amount to prevent bleed on the bottom of the pots/CVs
#define BOTTOM_THRESH 0.002f
using namespace daisy;

void AnalogControl::Init(uint16_t *adcptr,
                         float     sr,
                         bool      flip,
                         bool      invert,
                         float     slew_seconds)
{
    val_        = 0.0f;
    raw_        = adcptr;
    samplerate_ = sr;
    SetCoeff(1.0f / (slew_seconds * samplerate_ * 0.5f));
    scale_        = 1.0f;
    offset_       = 0.0f;
    flip_         = flip;
    invert_       = invert;
    is_bipolar_   = false;
    slew_seconds_ = slew_seconds;
}

void AnalogControl::InitBipolarCv(uint16_t *adcptr, float sr)
{
    val_        = 0.0f;
    raw_        = adcptr;
    samplerate_ = sr;
    SetCoeff(1.0f / (0.002f * samplerate_ * 0.5f));
    scale_      = 2.0f;
    offset_     = 0.5f;
    flip_       = false;
    invert_     = true;
    is_bipolar_ = true;
}

float AnalogControl::Process()
{
    float t;
    t = (float)*raw_ / 65536.0f;
    if(flip_)
        t = 1.f - t;
    t = (t - offset_) * scale_ * (invert_ ? -1.0f : 1.0f);
    val_ += coeff_ * (t - val_);
    return val_;
}

void AnalogControl::SetSampleRate(float sample_rate)
{
    samplerate_ = sample_rate;
    float slew  = is_bipolar_ ? .002f : slew_seconds_;
    SetCoeff(1.0f / (slew * samplerate_ * 0.5f));
}