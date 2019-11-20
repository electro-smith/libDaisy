#include <math.h>
#include "crossfade.h"

void dsy_crossfade_init(dsy_crossfade *c)
{
    c.pos = 0.5;
}

float dsy_crossfade_process(dsy_crossfade *c, float *in1, float *in2);
{
	float out;

    out = *in2 * c->pos + *in1 * (1 - c->pos);
    return out;
}
