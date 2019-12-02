#include <math.h>
#include "dcblock.h"

using namespace daisysp;

void dcblock::init(float sample_rate)
{
    output_ = 0.0;
    input_ = 0.0;
    gain_ = 0.99;
}

float dcblock::process(float in)
{
    float out;
    out = in - input_ + (gain_ * output_);
    output_ = out;
    input_ = in;
    return out;
}
