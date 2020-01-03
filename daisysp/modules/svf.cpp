#include <math.h>
#include "svf.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

using namespace daisysp;

void svf::init(float sample_rate)
{
	_sr = sample_rate;
	_fc = 200.0f;
	_res = 0.5f;
	_drive = 0.5f;
	_freq = 0.25f;
	_damp = 0.0f;
	_notch = 0.0f;
	_low = 0.0f;
	_high = 0.0f;
	_band = 0.0f;
	_peak = 0.0f;
	_input = 0.0f;
	_out_notch = 0.0f;
	_out_low = 0.0f;
	_out_high = 0.0f;
	_out_peak = 0.0f;
	_out_band = 0.0f;
}
void svf::process(float in)
{
	_input = in;
	// first pass
	_notch = _input - _damp * _band;
	_low = _low + _freq * _band;
	_high = _notch - _low;
	_band = _freq * _high + _band - _drive * _band * _band * _band;
	_out_low = 0.5f * _low;
	_out_high = 0.5f * _high;
	_out_band = 0.5f * _band;
	_out_peak = 0.5f * (_low - _high);
	_out_notch = 0.5f * _notch;
	// second pass
	_notch = _input - _damp * _band;
	_low = _low + _freq * _band;
	_high = _notch - _low;
	_band = _freq * _high + _band - _drive * _band * _band * _band;
	_out_low += 0.5f * _low;
	_out_high += 0.5f * _high;
	_out_band += 0.5f * _band;
	_out_peak += 0.5f * (_low - _high);
	_out_notch += 0.5f * _notch;
}

void svf::set_freq(float f)
{
	if (f < 0.000001f)
	{
		_fc = 0.000001f;
	}
	else if (f > _sr / 2.0f)
	{
		_fc = (_sr / 2.0f) - 1.0f;
	}
	else
	{
		_fc = f;
	}
	// Set Internal Frequency for _fc
	_freq = 2.0f * sinf((float)M_PI * MIN(0.25f, _fc / (_sr * 2.0f)));  // fs*2 because double sampled
	// recalculate damp
	//damp = (MIN(2.0f * powf(_res, 0.25f), MIN(2.0f, 2.0f / freq - freq * 0.5f)));
	_damp = MIN(2.0f*(1.0f - powf(_res, 0.25f)), MIN(2.0f, 2.0f / _freq - _freq * 0.5f));
}

void svf::set_res(float r)
{
	if (r < 0.0f)
	{
		r = 0.0f;
	}
	else if (r > 1.0f)
	{
		r = 1.0f;
	}
	_res = r;
	// recalculate damp
	//damp = (MIN(2.0f * powf(_res, 0.25f), MIN(2.0f, 2.0f / freq - freq * 0.5f)));
	_damp = MIN(2.0f*(1.0f - powf(_res, 0.25f)), MIN(2.0f, 2.0f / _freq - _freq * 0.5f));
}

