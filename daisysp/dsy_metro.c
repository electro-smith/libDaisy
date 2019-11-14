#include <math.h>
#include "dsy_metro.h"

void dsy_metro_init(dsy_metro_t *p, float freq, float sr) 
{
	p->freq = freq;
	p->phs  = 0.0f;
	p->sr   = sr;
	p->phs_inc = (2.0f * M_PI * p->freq) / p->sr;
}

void dsy_metro_set_freq(dsy_metro_t *p, float freq) 
{
	p->freq = freq;
	p->phs_inc = (2.0f * M_PI * p->freq) / p->sr;
}

uint8_t dsy_metro_process(dsy_metro_t *p) 
{
	p->phs += p->phs_inc;
	if(p->phs >= 2.0f * M_PI) 
	{
		p->phs -= (2.0f * M_PI);
		return 1;
	}
	return 0;
}
