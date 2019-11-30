#include <math.h>
#include "metro.h"

using namespace daisysp;

void metro::init(float freq, float sample_rate) 
{
	_freq = freq;
	_phs  = 0.0f;
	_sample_rate = sample_rate;
	_phs_inc = (2.0f * M_PI * _freq) / _sample_rate;
}

uint8_t metro::process() 
{
	_phs += _phs_inc;
	if(_phs >= 2.0f * M_PI) 
	{
		_phs -= (2.0f * M_PI);
		return 1;
	}
	return 0;
}
