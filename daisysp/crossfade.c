#include "crossfade.h"

void dsy_crossfade_init(dsy_crossfade *p, uint8_t curve)
{
    p->pos = 0.5;
    p->curve = curve;
}

float dsy_crossfade_process(dsy_crossfade *p, float *in1, float *in2)
{
	switch (p->curve)
	{
		case DSY_CROSSFADE_LIN: 
    		return ((*in2 * p->pos) + *in1) * (1 - p->pos);

    	case DSY_CROSSFADE_CPOW:
    		return 0;

    	case DSY_CROSSFADE_LOG:
    		return 0;

    	case DSY_CROSSFADE_EXP:
    		return 0;

    	default :
    		return 0;
	}
}

void dsy_crossfade_set_pos(dsy_crossfade *p, float pos) 
{
	p->pos = pos;
}

void dsy_crossfade_set_curve(dsy_crossfade *p, uint8_t curve)
{
	p->curve = curve;
}
