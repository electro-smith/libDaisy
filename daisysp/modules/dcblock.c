#include <math.h>
#include "dcblock.h"

void dsy_dcblock_init(dsy_dcblock_t *p, int sr)
{
    p->outputs = 0.0;
    p->inputs = 0.0;
    p->gain = 0.99;
}

float dsy_dcblock_process(dsy_dcblock_t *p, float in)
{
    float out;
    out = in - p->inputs + (p->gain * p->outputs);
    p->outputs = out;
    p->inputs = in;
    return out;
}
