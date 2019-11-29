#include <math.h>
#include "svf.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

using namespace daisysp;

void svf::init(float samplerate)
{
	sr = samplerate;
	fc = 200.0f;
	res = 0.5f;
	drive = 0.5f;
	freq = 0.25f;
	damp = 0.0f;
	pnotch = 0.0f;
	plow = 0.0f;
	phigh = 0.0f;
	pband = 0.0f;
	ppeak = 0.0f;
	input = 0.0f;
	out_notch = 0.0f;
	out_low = 0.0f;
	out_high = 0.0f;
	out_peak = 0.0f;
	out_band = 0.0f;
}
void svf::process(float in)
{
	input = in;
	// first pass
	pnotch = input - damp * pband;
	plow = plow + freq * pband;
	phigh = pnotch - plow;
	pband = freq * phigh + pband - drive * pband * pband * pband;
	out_low = 0.5f * plow;
	out_high = 0.5f * phigh;
	out_band = 0.5f * pband;
	out_peak = 0.5f * (plow - phigh);
	out_notch = 0.5f * pnotch;
	// second pass
	pnotch = input - damp * pband;
	plow = plow + freq * pband;
	phigh = pnotch - plow;
	pband = freq * phigh + pband - drive * pband * pband * pband;
	out_low += 0.5f * plow;
	out_high += 0.5f * phigh;
	out_band += 0.5f * pband;
	out_peak += 0.5f * (plow - phigh);
	out_notch += 0.5f * pnotch;
}

void svf::set_freq(float f)
{
	if (f < 0.000001f)
	{
		fc = 0.000001f;
	}
	else if (f > sr / 2.0f)
	{
		fc = (sr / 2.0f) - 1.0f;
	}
	else
	{
		fc = f;
	}
	// Set Internal Frequency for fc
	freq = 2.0f * sinf((float)M_PI * MIN(0.25f, fc / (sr * 2.0f)));  // fs*2 because double sampled
	// recalculate damp
	//damp = (MIN(2.0f * powf(res, 0.25f), MIN(2.0f, 2.0f / freq - freq * 0.5f)));
	damp = MIN(2.0f*(1.0f - powf(res, 0.25f)), MIN(2.0f, 2.0f / freq - freq * 0.5f));
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
	res = r;
	// recalculate damp
	//damp = (MIN(2.0f * powf(res, 0.25f), MIN(2.0f, 2.0f / freq - freq * 0.5f)));
	damp = MIN(2.0f*(1.0f - powf(res, 0.25f)), MIN(2.0f, 2.0f / freq - freq * 0.5f));
}

