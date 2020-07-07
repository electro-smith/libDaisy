#include "hid/parameter.h"
#include <math.h>

using namespace daisy;

void Parameter::Init(AnalogControl input, float min, float max, Curve curve)
{
    pmin_   = min;
    pmax_   = max;
    pcurve_ = curve;
    in_     = input;
    lmin_   = logf(min < 0.0000001f ? 0.0000001f : min);
    lmax_   = logf(max);
}

float Parameter::Process()
{
    switch(pcurve_)
    {
        case LINEAR: val_ = (in_.Process() * (pmax_ - pmin_)) + pmin_; break;
        case EXPONENTIAL:
            val_ = in_.Process();
            val_ = ((val_ * val_) * (pmax_ - pmin_)) + pmin_;
            break;
        case LOGARITHMIC:
            val_ = expf((in_.Process() * (lmax_ - lmin_)) + lmin_);
            break;
        case CUBE:
            val_ = in_.Process();
            val_ = ((val_ * (val_ * val_)) * (pmax_ - pmin_)) + pmin_;
            break;
        default: break;
    }
    return val_;
}