#include "line.h"
void dsy_line_init(dsy_line *p, float sr) 
{
	p->sr	= sr;
	p->dur = 0.5f;
	p->end   = 0.0f;
	p->start = 1.0f;
	p->val   = 1.0f;
}
void dsy_line_start(dsy_line *p, float start, float end, float dur)
{
	p->start = start;
	p->end   = end;
	p->dur   = dur;
	p->inc = (end - start) / ((p->sr * p->dur));
	p->val = p->start;
	p->finished = 0;
}

float dsy_line_process(dsy_line *p, uint8_t *finished)
{
	float out;
	out = p->val;

	if((p->end > p->start && out >= p->end)
	   || (p->end < p->start && out <= p->end))
	{
		p->finished = 1;
		p->val		= p->end;
		out			= p->end;
	}
	else
	{
		p->val += p->inc;
	}
	*finished = p->finished;
	return out;
}
