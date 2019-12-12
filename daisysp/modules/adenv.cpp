#include <math.h>
#include "adenv.h"

using namespace daisysp;

// Private Functions
void adenv::init(float sample_rate)
{
	sample_rate_ = sample_rate;
	current_segment_ = ADENV_SEG_IDLE;
	curve_scalar_ = 0.0f;  // full linear
	phase_ = 0;
	multiplier_ = 1.0f;
	//phase__inc_ = ((2.0f * PI * (1.0f / segment_time_[ADENV_SEG_ATTACK]) / SAMPLE_RATE));// / 2.0f);
	phase_inc_ = 1.0f;
	min_ = 0.0f;
	max_ = 1.0f;
	output_ = 0.0001f;
	for (uint8_t i = 0; i < ADENV_SEG_LAST; i++) {
		segment_time_[i] = 0.05f;
	}	
}

float adenv::process()
{
	uint32_t time_samps;
	if (trigger_)
	{
		trigger_ = 0;
		current_segment_ = ADENV_SEG_ATTACK;
		phase_ = 0;
        time_samps = (uint32_t)(segment_time_[current_segment_] * sample_rate_);
        calculate_multiplier(output_, 1.0f, time_samps);
	}
    else
    {
        time_samps = (uint32_t)(segment_time_[current_segment_] * sample_rate_);
    }
	if (phase_ >= time_samps)
	{
		switch (current_segment_)	
		{
		case ADENV_SEG_ATTACK:
			current_segment_ = ADENV_SEG_DECAY;
			time_samps = (uint32_t)(segment_time_[current_segment_] * sample_rate_);
			phase_ = 0;
			output_ = 1.0f;
			calculate_multiplier(1.0f, 0.0001f, time_samps);
			break;
		case ADENV_SEG_DECAY:
			current_segment_ = ADENV_SEG_IDLE;
			phase_ = 0;
			multiplier_ = 1.0f;
			output_ = 0.0f;
			break;
		default:
			multiplier_ = 0.0f;		
			output_ = 0.0f;
			phase_ = 0;
			break;
		}
	}
	output_ *= multiplier_;
	phase_ += 1;
	return output_ * (max_ - min_) + min_;
}

void adenv::calculate_multiplier(float start, float end, uint32_t length_in_samples) 
{
	multiplier_ = 1.0f + ((end - start) / (float)length_in_samples);
}
