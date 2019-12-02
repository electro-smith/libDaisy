#include <math.h>
#include "phasor.h"

using namespace daisysp;

void phasor::set_freq(float freq) 
{
	freq_ = freq;
	inc_  = (2.0f * M_PI * freq_) / sample_rate_;
}

float phasor::process() 
{
	float out;
	out = phs_ / (2.0f * M_PI);
	phs_ += inc_;
	if(phs_ > 2.0f * M_PI) 
	{
		phs_ -= 2.0f * M_PI;
	}
	if(phs_ < 0.0f)
	{
		phs_ = 0.0f;
	}
	return out;
}
