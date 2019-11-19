#include <math.h>
#include "phasor.h"

void dsy_phasor_init(dsy_phasor_t *p, float sr, float freq, float initial_phase) 
{
	p->sr = sr;
	p->phs = initial_phase;
	dsy_phasor_set_freq(p, freq);
}
void dsy_phasor_set_freq(dsy_phasor_t *p, float freq) 
{
	p->freq = freq;
	p->inc  = (2.0f * M_PI * p->freq) / p->sr;
}
float dsy_phasor_process(dsy_phasor_t *p) 
{
	float out;
	out = p->phs / (2.0f * M_PI);
	p->phs += p->inc;
	if(p->phs > 2.0f * M_PI) 
	{
		p->phs -= 2.0f * M_PI;
	}
	if(p->phs < 0.0f)
	{
		p->phs = 0.0f;
	}
	return out;
}
