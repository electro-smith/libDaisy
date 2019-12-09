//#include <math.h>
#include <cmath>
#include <stdlib.h>
#include <stdint.h>
#include "compressor.h"

using namespace daisysp;

// From faust args are:
// 0 - atk
// 1 - ratio
// 2 - rel
// 3 - thresh

#ifndef max
#define max(a,b) ((a < b) ? b : a)
#endif

#ifndef min
#define min(a,b) ((a < b) ? a : b)
#endif

void compressor::init(float samplerate)
{
    sr_ = samplerate;
    iConst0_ = min(192000, max(1, sr_));
    fConst1_ = 2.0f / (float)iConst0_;
    fConst2_ = 1.0f / (float) iConst0_; 
    // Skipped fHsliderN inits, but I'm going to init the 4 params
    ratio_ = 2.0f;
    thresh_ = -12.0f;
    atk_ = 0.1f;
    rel_ = 0.1f; 
    for(uint8_t i = 0; i < 2; i++)
    {
        fRec0_[i] = 0.1f;
        fRec1_[i] = 0.1f;
        fRec2_[i] = 0.1f;
    }

}

//float compressor::process(const float &in)
float compressor::process(float &in, float &key)
{
    float out;
	// Makeup gain may still be a little hot.
    float fTemp1 = fabsf(key);
    float fTemp2 = ((fRec1_[1] > fTemp1) ? fSlow4_ : fSlow3_);
    fRec2_[0] = ((fRec2_[1]  * fTemp2) + ((1.0f - fTemp2) * fTemp1));
    fRec1_[0] = fRec2_[0];
	fRec0_[0]
		= ((fSlow1_ * fRec0_[1])
		   + (fSlow2_ * max(((20.f * std::log10(fRec1_[0])) - fSlow5_), 0.f)));
    out = (powf(10.0f, (0.05f * fRec0_[0] * (0.05f * makeupgain_))) * in);
	fRec2_[1] = fRec2_[0];
    fRec1_[1] = fRec1_[0];
    fRec0_[1] = fRec0_[0];
    return out;
}

// Same without sidechain
float compressor::process(float &in) 
{
	return process(in, in);
}

void compressor::recalculate_slopes() 
{
	makeupgain_ = fabsf(thresh_ / ratio_) / 2.0f;
	fSlow0_ = (float)atk_; // probably need to scale
    fSlow1_ = expf(0.0f - (fConst1_ / fSlow0_));
    fSlow2_ = ((1.0f - fSlow1_) * ((1.0f / ratio_) - 1.0f));
    fSlow3_ = expf((0.0f - (fConst2_ / fSlow0_)));
    fSlow4_ = expf((0.0f - (fConst2_ / rel_)));
    fSlow5_ = thresh_;
}
