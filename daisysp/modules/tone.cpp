#include "tone.h"
#include "audio.h"
#include "arm_math.h"

using namespace daisysp;

void tone::init(float sample_rate)
{
    prevout_ = 0.0f;
    freq_ = 100.0f;
    c1_ = 0.5f;
    c2_ = 0.5f;
    sample_rate_ = sample_rate;
}

float tone::process(float *in)
{
    float out;

    out = c1_ * *in + c2_ * prevout_;
    prevout_ = out;
    
    return out;
}

void tone::set_freq(float freq)
{
    freq_ = freq;
    calculate_coefficients();
}

void tone::calculate_coefficients()
{
    float b, c1, c2;
    b = 2.0f - cosf(2.0f * M_PI * freq_ / sample_rate_);
    c2 = b - sqrtf(b * b - 1.0f);
    c1 = 1.0f - c2;
    c1_ = c1;
    c2_ = c2;
}
