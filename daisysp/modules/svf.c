#include <math.h>
#include "svf.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void dsy_svf_init(dsy_svf *f, float sample_rate)
{
	f->sr = sample_rate;
	f->fc = 200.0f;
	f->res = 0.5f;
	f->drive = 0.5f;
	f->freq = 0.25f;
	f->damp = 0.0f;
	f->notch = 0.0f;
	f->low = 0.0f;
	f->high = 0.0f;
	f->band = 0.0f;
	f->peak = 0.0f;
	f->input = 0.0f;
	f->out_notch = 0.0f;
	f->out_low = 0.0f;
	f->out_high = 0.0f;
	f->out_peak = 0.0f;
	f->out_band = 0.0f;
}
void dsy_svf_process(dsy_svf *f, float *in)
{
	f->input = *in;
	// first pass
	f->notch = f->input - f->damp * f->band;
	f->low = f->low + f->freq * f->band;
	f->high = f->notch - f->low;
	f->band = f->freq * f->high + f->band - f->drive * f->band * f->band * f->band;
	f->out_low = 0.5f * f->low;
	f->out_high = 0.5f * f->high;
	f->out_band = 0.5f * f->band;
	f->out_peak = 0.5f * (f->low - f->high);
	f->out_notch = 0.5f * f->notch;
	// second pass
	f->notch = f->input - f->damp * f->band;
	f->low = f->low + f->freq * f->band;
	f->high = f->notch - f->low;
	f->band = f->freq * f->high + f->band - f->drive * f->band * f->band * f->band;
	f->out_low += 0.5f * f->low;
	f->out_high += 0.5f * f->high;
	f->out_band += 0.5f * f->band;
	f->out_peak += 0.5f * (f->low - f->high);
	f->out_notch += 0.5f * f->notch;
}
// setters
void dsy_svf_set_fc(dsy_svf *f, float frequency)
{
	if (frequency < 0.000001f)
	{
		f->fc = 0.000001f;
	}
	else if (frequency > f->sr / 2.0f)
	{
		f->fc = (f->sr / 2.0f) - 1.0f;
	}
	else
	{
		f->fc = frequency;
	}
	// Set Internal Frequency for fc
	f->freq = 2.0f * sinf((float)M_PI * MIN(0.25f, f->fc / (f->sr * 2.0f)));  // fs*2 because double sampled
	// recalculate damp
	//f->damp = (MIN(2.0f * powf(f->res, 0.25f), MIN(2.0f, 2.0f / f->freq - f->freq * 0.5f)));
	f->damp = MIN(2.0f*(1.0f - powf(f->res, 0.25f)), MIN(2.0f, 2.0f / f->freq - f->freq * 0.5f));
}
void dsy_svf_set_res(dsy_svf *f, float res)
{
	if (res < 0.0f)
	{
		res = 0.0f;
	}
	else if (res > 1.0f)
	{
		res = 1.0f;
	}
	f->res = res;
	// recalculate damp
	//f->damp = (MIN(2.0f * powf(res, 0.25f), MIN(2.0f, 2.0f / f->freq - f->freq * 0.5f)));
	f->damp = MIN(2.0f*(1.0f - powf(f->res, 0.25f)), MIN(2.0f, 2.0f / f->freq - f->freq * 0.5f));
}
void dsy_svf_set_drive(dsy_svf *f, float drive)
{
	if (drive < 0.0f)
	{
		drive = 0.0f;
	}
	else if (drive > 1.0f)
	{
		drive = 1.0f;
	}
	f->drive = drive;
}
// outs
float dsy_svf_notch(dsy_svf *f)
{
	return f->out_notch;
}
float dsy_svf_low(dsy_svf *f)
{
	return f->out_low;
}
float dsy_svf_high(dsy_svf *f)
{
	return f->out_high;
}
float dsy_svf_band(dsy_svf *f)
{
	return f->out_band;
}
float dsy_svf_peak(dsy_svf *f)
{
	return f->out_peak;
}
