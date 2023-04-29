#include "hid/ctrl.h"
#include <math.h>
// Temporary amount to prevent bleed on the bottom of the pots/CVs
#define BOTTOM_THRESH 0.002f
using namespace daisy;

void AnalogControl::Init(uint16_t* adcptr,
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

void AnalogControl::InitBipolarCv(uint16_t* adcptr, float sr)
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

extern "C" dsy_analog_ctrl* dsy_ctrl_new()
{
    return (dsy_analog_ctrl*)static_cast<void*>(new AnalogControl());
}

extern "C" void dsy_ctrl_destroy(dsy_analog_ctrl* ctrl)
{
    delete static_cast<AnalogControl*>((void*)ctrl);
}

extern "C" void dsy_ctrl_init(dsy_analog_ctrl* ctrl,
                              uint16_t*        adcptr,
                              float            sr,
                              bool             flip,
                              bool             invert,
                              float            slew_seconds)
{
    static_cast<AnalogControl*>((void*)ctrl)
        ->Init(adcptr, sr, flip, invert, slew_seconds);
}

extern "C" void
dsy_init_bipolar_cv(dsy_analog_ctrl* ctrl, uint16_t* adcptr, float sr)
{
    static_cast<AnalogControl*>((void*)ctrl)->InitBipolarCv(adcptr, sr);
}

extern "C" float dsy_ctrl_process(dsy_analog_ctrl* ctrl)
{
    return static_cast<AnalogControl*>((void*)ctrl)->Process();
}

extern "C" float dsy_ctrl_value(dsy_analog_ctrl* ctrl)
{
    return static_cast<AnalogControl*>((void*)ctrl)->Value();
}

extern "C" void dsy_ctrl_set_coeff(dsy_analog_ctrl* ctrl, float val)
{
    return static_cast<AnalogControl*>((void*)ctrl)->SetCoeff(val);
}

extern "C" uint16_t dsy_ctrl_get_raw_value(dsy_analog_ctrl* ctrl)
{
    return static_cast<AnalogControl*>((void*)ctrl)->GetRawValue();
}

extern "C" float dsy_ctrl_get_raw_float(dsy_analog_ctrl* ctrl)
{
    return static_cast<AnalogControl*>((void*)ctrl)->GetRawFloat();
}

extern "C" void dsy_ctrl_set_sample_rate(dsy_analog_ctrl* ctrl, float sr)
{
    static_cast<AnalogControl*>((void*)ctrl)->SetSampleRate(sr);
}
