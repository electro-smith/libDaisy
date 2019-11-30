#include <math.h>
#include "phasor.h"

using namespace daisysp;

float phasor::process() 
{
	float out;
	out = _phs / (2.0f * M_PI);
	_phs += _inc;
	if(_phs > 2.0f * M_PI) 
	{
		_phs -= 2.0f * M_PI;
	}
	if(_phs < 0.0f)
	{
		_phs = 0.0f;
	}
	return out;
}
