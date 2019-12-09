#include <math.h>
#include "phasor.h"

using namespace daisysp;

void phasor::set_freq(float freq) 
{
	freq_ = freq;
	inc_  = ((float)M_TWOPI * freq_) / sample_rate_;
}

float phasor::process() 
{
	float out;
	out = phs_ / ((float)M_TWOPI);
	phs_ += inc_;
	if(phs_ > (float)M_TWOPI)
	{
		phs_ -= (float)M_TWOPI;
	}
	if(phs_ < 0.0f)
	{
		phs_ = 0.0f;
	}
	return out;
}
