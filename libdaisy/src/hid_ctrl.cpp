#include "hid_ctrl.h"
#include <math.h>
// Temporary amount to prevent bleed on the bottom of the pots/CVs
#define BOTTOM_THRESH 0.002f
using namespace daisy;

void AnalogControl::Init(uint16_t *adcptr, float sr)
// ~~~~
{
	val_		= 0.0f;
	raw_		= adcptr;
	samplerate_ = sr;
	coeff_		= 1.0f / (0.002f * samplerate_ * 0.5f);
	scale_		= 1.0f;
	offset_		= 0.0f;
	flip_		= false;
}

// ~~~~
void AnalogControl::Init(uint16_t *adcptr, float sr, float slew_seconds)
// ~~~~
{
	val_		= 0.0f;
	raw_		= adcptr;
	samplerate_ = sr;
	coeff_		= 1.0f / (slew_seconds * samplerate_ * 0.5f);
	scale_		= 1.0f;
	offset_		= 0.0f;
	flip_		= false;
}

void AnalogControl::InitBipolarCv(uint16_t *adcptr, float sr)
{
	val_		= 0.0f;
	raw_		= adcptr;
	samplerate_ = sr;
	coeff_		= 1.0f / (0.002f * samplerate_ * 0.5f);
	scale_  = 2.0f;
	offset_ = 0.5f;
	flip_   = true;
}

float AnalogControl::Process() { 
	float t;
	t = (float)*raw_ / 65536.0f;
	t = (t - offset_) * scale_ * (flip_ ? -1.0f : 1.0f);
	if(t < BOTTOM_THRESH)
		t = 0.0f;
	val_ += coeff_ * (t - val_);
	return val_;
}
