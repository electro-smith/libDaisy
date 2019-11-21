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
		case DSY_CROSSFADE_LINEAR: 
    		return ((*in2 * p->pos) + *in1) * (1 - p->pos);

    	default :
    		return 0;

	}
}
