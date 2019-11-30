#include <math.h>
#include "dcblock.h"

using namespace daisysp;

void dcblock::init(int sample_rate)
{
    _outputs = 0.0;
    _inputs = 0.0;
    _gain = 0.99;
}

float dcblock::process(float in)
{
    float out;
    out = in - _inputs + (_gain * _outputs);
    _outputs = out;
    _inputs = in;
    return out;
}
