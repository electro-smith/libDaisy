#include <math.h>
#include "adenv.h"

// Private Functions
static void calculate_multiplier(dsy_adenv *e, float start, float end, uint32_t length_in_samples);
void dsy_adenv_init(dsy_adenv *p, float sr)
{
	p->sr = sr;
	p->current_segment = DSY_ADENV_SEG_IDLE;
	p->curve_scalar = 0.0f;  // full linear
	p->phase = 0;
	p->multiplier = 1.0f;
	//p->phase_inc = ((2.0f * PI * (1.0f / p->segment_time[ADENV_SEG_RISE]) / SAMPLE_RATE));// / 2.0f);
	p->phase_inc = 1.0f;
	p->min = 0.0f;
	p->max = 1.0f;
	p->output = 0.0001f;
	for (uint8_t i = 0; i < DSY_ADENV_SEG_LAST; i++) {
		p->segment_time[i] = 0.05f;
	}	
}
void dsy_adenvrigger(dsy_adenv *p)
{
	p->trigger = 1;		
}
void dsy_adenv_set_segment_time(dsy_adenv *p, dsy_adenv_segment seg, float time)
{
	p->segment_time[seg] = time;
}
void dsy_adenv_set_curve_scalar(dsy_adenv *p, float scalar)
{
	p->curve_scalar = scalar;	
}
void dsy_adenv_set_min_max(dsy_adenv *p, float min, float max)
{
	p->min = min;
	p->max = max;
}

float dsy_adenv_process(dsy_adenv *p)
{
	uint32_t time_samps;
	if (p->trigger)
	{
		p->trigger = 0;
		p->current_segment = DSY_ADENV_SEG_RISE;
		p->phase = 0;
        time_samps = (uint32_t)(p->segment_time[p->current_segment] * p->sr);
        calculate_multiplier(p, p->output, 1.0f, time_samps);
	}
    else
    {
        time_samps = (uint32_t)(p->segment_time[p->current_segment] * p->sr);
    }
	if (p->phase >= time_samps)
	{
		switch (p->current_segment)	
		{
		case DSY_ADENV_SEG_RISE:
			p->current_segment = DSY_ADENV_SEG_FALL;
			time_samps = (uint32_t)(p->segment_time[p->current_segment] * p->sr);
			p->phase = 0;
			p->output = 1.0f;
			calculate_multiplier(p, 1.0f, 0.0001f, time_samps);
			break;
		case DSY_ADENV_SEG_FALL:
			p->current_segment = DSY_ADENV_SEG_IDLE;
			p->phase = 0;
			p->multiplier = 1.0f;
			p->output = 0.0f;
			break;
		default:
			p->multiplier = 0.0f;
			p->output = 0.0f;
			p->phase = 0;
			break;
		}
	}
	p->output *= p->multiplier;
	p->phase += 1;
	return p->output * (p->max - p->min) + p->min;
}

static void calculate_multiplier(dsy_adenv *p, float start, float end, uint32_t length_in_samples) 
{
	p->multiplier = 1.0f + ((end - start) / (float)length_in_samples);
	//= 1.0f + ((logf(end) - logf(start)) / (float)length_in_samples);
}
