#include "dsy_oscillator.h"
#include <math.h>
static float polyblep(float phase_inc, float t);
void dsy_oscillator_init(dsy_oscillator_t *p, float sr)
{
	p->sr = sr;
	p->freq = 100.0f;	
	p->amp = 1.0f;
	p->phase = 0.0f;
	p->wave = DSY_OSC_WAVE_SIN;
	p->phase_inc = ((2.0f * (float)M_PI * p->freq) / p->sr);
	p->last_out = 0.0f;
	p->last_freq = p->freq;
}
float dsy_oscillator_process(dsy_oscillator_t *p)
{
	float out;
	if (p->last_freq != p->freq)
	{
		p->last_freq = p->freq;	
		// recompute phase inc only when freq changes
		p->phase_inc = ((2.0f * (float)M_PI * p->freq) / p->sr);
	}
	switch (p->wave)
	{
	case DSY_OSC_WAVE_SIN:
		out = sinf(p->phase);	
		break;
	case DSY_OSC_WAVE_TRI:
		{
			float val;
			val = -1.0f + (2.0f * p->phase / (2.0f * M_PI));
			out = (2.0f * (fabsf(val) - 0.5f));
		}
		break;
	case DSY_OSC_WAVE_SAW:
		out = (1.0f - (p->phase / (2.0f * M_PI)));
		break;
	case DSY_OSC_WAVE_SQUARE:
		out = p->phase < M_PI ? (1.0f) :  -1.0f;
		break;
	case DSY_OSC_WAVE_POLYBLEP_TRI:
		{
			float t;
			t = p->phase / (2.0f * (float)M_PI);
			out = p->phase < (float)M_PI ? 1.0f : -1.0f;
			out += polyblep(p->phase_inc, t);
			out -= polyblep(p->phase_inc, fmodf(t + 0.5f, 1.0f));
			// Leaky Integrator:
			// y[n] = A + x[n] + (1 - A) * y[n-1]
			out = p->phase_inc * out + (1.0f - p->phase_inc) * p->last_out;
		}
		break;
	case DSY_OSC_WAVE_POLYBLEP_SAW:
		{
			float t;
			t = p->phase / (2.0f * (float)M_PI);
			out = (2.0f * p->phase / (2.0f * (float)M_PI)) - 1.0f;
			out -= polyblep(p->phase_inc, t);
			out *=  -1.0f;
		}
		break;
	case DSY_OSC_WAVE_POLYBLEP_SQUARE:
		{
			float t;	
			t = p->phase / (2.0f * (float)M_PI);
			out = p->phase < M_PI ? 1.0f : -1.0f;
			out += polyblep(p->phase_inc, t);
			out -= polyblep(p->phase_inc, fmodf(t + 0.5f, 1.0f));
			out *= 0.707f; // ?
		}
		break;
	default:
	break;
	}
	p->last_out = out;
	p->phase += p->phase_inc;
	if (p->phase > 2.0f * (float)M_PI)
	{
		p->phase -= (2.0f *(float)M_PI);
	}
	return out * p->amp;
}
void dsy_oscillator_reset_phase(dsy_oscillator_t *p)
{
	p->phase = 0.0f;
}

static float polyblep(float phase_inc, float t)
{
	float dt = phase_inc / (2.0f * (float)M_PI);
	if (t < dt)
	{
		t /= dt;
		return t + t - t * t - 1.0f;
	}
	else if (t > 1.0f - dt)
	{
		t = (t - 1.0f) / dt;
		return t * t + t + t + 1.0f;
	}
	else
	{
		return 0.0f;
	}
}
