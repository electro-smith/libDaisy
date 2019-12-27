#include <math.h>
#include "oscillator.h"
// TODO: Fix polyblep triangle... something bad happened to it.
using namespace daisysp;
static inline float polyblep(float phase_inc, float t);
float oscillator::process() 
{
	float out, t;
	switch(waveform)
	{
		case WAVE_SIN: out = sinf(phase_); break;
		case WAVE_TRI: 
			t = -1.0f + (2.0f * phase_ / (2.0f * M_PI));
			out = 2.0f * (fabsf(t) - 0.5f);
			break;
		case WAVE_SAW:
			out = -1.0f * (((phase_ / (2.0f * M_PI) * 2.0f)) - 1.0f);
			break;
		case WAVE_RAMP:
			out = ((phase_ / (2.0f * M_PI) * 2.0f)) - 1.0f;
			break;
		case WAVE_SQUARE:
			out = phase_ < M_PI ? (1.0f) :  -1.0f;
			break;
		case WAVE_POLYBLEP_TRI:
			t = phase_ / (2.0f * (float)M_PI);
			out = phase_ < (float)M_PI ? 1.0f : -1.0f;
			out += polyblep(phase_inc, t);
			out -= polyblep(phase_inc, fmodf(t + 0.5f, 1.0f));
			// Leaky Integrator:
			// y[n] = A + x[n] + (1 - A) * y[n-1]
			out = phase_inc * out + (1.0f - phase_inc) * last_out;
			break;
		case WAVE_POLYBLEP_SAW:
			t = phase_ / (2.0f * (float)M_PI);
			out = (2.0f * phase_ / (2.0f * (float)M_PI)) - 1.0f;
			out -= polyblep(phase_inc, t);
			out *=  -1.0f;
			break;
		case WAVE_POLYBLEP_SQUARE:
			t = phase_ / (2.0f * (float)M_PI);
			out = phase_ < M_PI ? 1.0f : -1.0f;
			out += polyblep(phase_inc, t);
			out -= polyblep(phase_inc, fmodf(t + 0.5f, 1.0f));
			out *= 0.707f; // ?
			break;
		default:
			out = 0.0f;
			break;
	}
	phase_ += phase_inc;
	if(phase_ > (2.0f * M_PI)) 
	{
		phase_ -= (2.0f * M_PI);
	}
	return out * amp;
}
float oscillator::calc_phase_inc(float f) {
	return ((2.0f * (float)M_PI * f) / sr);
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
//static float polyblep(float phase_inc, float t);
//void dsy_oscillator_init(dsy_oscillator *p, float sr)
//{
//	sr = sr;
//	freq = 100.0f;	
//	amp = 1.0f;
//	phase_ = 0.0f;
//	wave = WAVE_SIN;
//	phase_inc = ((2.0f * (float)M_PI * p->freq) / p->sr);
//	last_out = 0.0f;
//	last_freq = p->freq;
//}
//float dsy_oscillator_process(dsy_oscillator *p)
//{
//	float out;
//	if (last_freq != p->freq)
//	{
//		last_freq = p->freq;	
//		// recompute phase_ inc only when freq changes
//		phase_inc = ((2.0f * (float)M_PI * p->freq) / p->sr);
//	}
//	switch (wave)
//	{
//	case WAVE_SIN:
//		out = sinf(phase_);	
//		break;
//	case WAVE_TRI:
//		{
//			float val;
//			val = -1.0f + (2.0f * phase_ / (2.0f * M_PI));
//			out = (2.0f * (fabsf(val) - 0.5f));
//		}
//		break;
//	case WAVE_SAW:
//		out = -1.0f * (((phase_ / (2.0f * M_PI) * 2.0f)) - 1.0f);
//		break;
//	case WAVE_RAMP:
//		out = ((phase_ / (2.0f * M_PI) * 2.0f)) - 1.0f;
//		break;
//	case WAVE_SQUARE:
//		out = phase_ < M_PI ? (1.0f) :  -1.0f;
//		break;
//	case WAVE_POLYBLEP_TRI:
//		{
//			float t;
//			t = phase_ / (2.0f * (float)M_PI);
//			out = phase_ < (float)M_PI ? 1.0f : -1.0f;
//			out += polyblep(phase_inc, t);
//			out -= polyblep(phase_inc, fmodf(t + 0.5f, 1.0f));
//			// Leaky Integrator:
//			// y[n] = A + x[n] + (1 - A) * y[n-1]
//			out = phase_inc * out + (1.0f - p->phase_inc) * p->last_out;
//		}
//		break;
//	case WAVE_POLYBLEP_SAW:
//		{
//			float t;
//			t = phase_ / (2.0f * (float)M_PI);
//			out = (2.0f * phase_ / (2.0f * (float)M_PI)) - 1.0f;
//			out -= polyblep(phase_inc, t);
//			out *=  -1.0f;
//		}
//		break;
//	case WAVE_POLYBLEP_SQUARE:
//		{
//			float t;	
//			t = phase_ / (2.0f * (float)M_PI);
//			out = phase_ < M_PI ? 1.0f : -1.0f;
//			out += polyblep(phase_inc, t);
//			out -= polyblep(phase_inc, fmodf(t + 0.5f, 1.0f));
//			out *= 0.707f; // ?
//		}
//		break;
//	default:
//        out = 0.0f;
//        break;
//	}
//	last_out = out;
//	phase_ += p->phase_inc;
//	if (phase_ > 2.0f * (float)M_PI)
//	{
//		phase_ -= (2.0f *(float)M_PI);
//	}
//	return out * amp;
//}
//void dsy_oscillator_reset_phase_(dsy_oscillator *p)
//{
//	phase_ = 0.0f;
//}
//
