#include <math.h>
#include "adenv.h"

using namespace daisysp;

#define EXPF expf_fast

// Fast Exp approximation
inline float expf_fast(float x)
{
   x = 1.0f + x / 256;
   x *= x;
   x *= x; 
   x *= x;
   x *= x; 
   x *= x;
   x *= x; 
   x *= x;
   x *= x;
   return x;
}

// Private Functions
void adenv::init(float sample_rate)
{
    sample_rate_     = sample_rate;
    current_segment_ = ADENV_SEG_IDLE;
    curve_scalar_    = 0.0f; // full linear
    phase_           = 0;
    multiplier_      = 1.0f;
    //phase__inc_ = ((2.0f * PI * (1.0f / segment_time_[ADENV_SEG_ATTACK]) / SAMPLE_RATE));// / 2.0f);
    phase_inc_ = 1.0f;
    min_       = 0.0f;
    max_       = 1.0f;
    output_    = 0.0001f;
    for(uint8_t i = 0; i < ADENV_SEG_LAST; i++)
    {
        segment_time_[i] = 0.05f;
    }
}

float adenv::process()
{
    uint32_t time_samps;
    float    val, out, end, beg, inc;

    // Handle Retriggering
    if(trigger_)
    {
        trigger_         = 0;
        current_segment_ = ADENV_SEG_ATTACK;
        phase_           = 0;
        curve_x_         = 0.0f;
        retrig_val_      = output_;
    }

    time_samps = (uint32_t)(segment_time_[current_segment_] * sample_rate_);

    // Fixed for now, but we could always make this a more flexible multi-segment envelope
    switch(current_segment_)
    {
        case ADENV_SEG_ATTACK:
            beg = retrig_val_;
            end = 1.0f;
            break;
        case ADENV_SEG_DECAY:
            beg = 1.0f;
            end = 0.0f;
            break;
        case ADENV_SEG_IDLE:
        default:
            beg = 0;
            end = 0;
            break;
    }

    if(prev_segment_ != current_segment_)
    {
        // Handle recalculating things and whatnot
        curve_x_ = 0;
        phase_   = 0;
        // This can happen only on transitions between curves
        if(curve_scalar_ == 0.0f)
        {
            c1_ = (end - beg) / time_samps;
        }
        else
        {
            c1_ = (end - beg) / (1.0f - EXPF(curve_scalar_));
        }
    }

    // update output
    val = output_;
    inc = c1_;
    out = val;
    if(curve_scalar_ == 0.0f)
    {
        val += inc;
    }
    else
    {
        curve_x_ += (curve_scalar_ / time_samps);
        val = beg + inc * (1.0f - EXPF(curve_x_));
    }

    // Update Segment
    phase_ += 1;
    prev_segment_ = current_segment_;
    if(phase_ > time_samps && current_segment_ != ADENV_SEG_IDLE)
    {
        // Advance segment
        current_segment_++;
        // TODO: Add Cycling feature here.
        if(current_segment_ > ADENV_SEG_DECAY)
        {
            current_segment_ = ADENV_SEG_IDLE;
        }
    }

    output_ = val;

    return out * (max_ - min_) + min_;
}

void adenv::calculate_multiplier(float    start,
                                 float    end,
                                 uint32_t length_in_samples)
{
    multiplier_ = 1.0f + ((end - start) / (float)length_in_samples);
}
