#include "line.h"

using namespace daisysp;

void line::init(float sr) 
{
	_sr	= sr;
	_dur = 0.5f;
	_end   = 0.0f;
	_start = 1.0f;
	_val   = 1.0f;
}
void line::start(float start, float end, float dur)
{
	_start = start;
	_end   = end;
	_dur   = dur;
	_inc = (end - start) / ((_sr * _dur));
	_val = _start;
	_finished = 0;
}

float line::process(uint8_t *finished)
{
	float out;
	out = _val;

	if((_end > _start && out >= _end)
	   || (_end < _start && out <= _end))
	{
		_finished = 1;
		_val		= _end;
		out			= _end;
	}
	else
	{
		_val += _inc;
	}
	*finished = _finished;
	return out;
}
