#include <math.h>
#include "adenv.h"

using namespace daisysp;

// Private Functions
void adenv::init(float sample_rate)
{
	_sr = sample_rate;
	_current_segment = ADENV_SEG_IDLE;
	_curve_scalar = 0.0f;  // full linear
	_phase = 0;
	_multiplier = 1.0f;
	//__phase_inc = ((2.0f * PI * (1.0f / _segment_time[ADENV_SEG_ATTACK]) / SAMPLE_RATE));// / 2.0f);
	_phase_inc = 1.0f;
	_min = 0.0f;
	_max = 1.0f;
	_output = 0.0001f;
	for (uint8_t i = 0; i < ADENV_SEG_LAST; i++) {
		_segment_time[i] = 0.05f;
	}	
}

float adenv::process()
{
	uint32_t time_samps;
	if (_trigger)
	{
		_trigger = 0;
		_current_segment = ADENV_SEG_ATTACK;
		_phase = 0;
        time_samps = (uint32_t)(_segment_time[_current_segment] * _sr);
        calculate_multiplier(_output, 1.0f, time_samps);
	}
    else
    {
        time_samps = (uint32_t)(_segment_time[_current_segment] * _sr);
    }
	if (_phase >= time_samps)
	{
		switch (_current_segment)	
		{
		case ADENV_SEG_ATTACK:
			_current_segment = ADENV_SEG_DECAY;
			time_samps = (uint32_t)(_segment_time[_current_segment] * _sr);
			_phase = 0;
			_output = 1.0f;
			calculate_multiplier(1.0f, 0.0001f, time_samps);
			break;
		case ADENV_SEG_DECAY:
			_current_segment = ADENV_SEG_IDLE;
			_phase = 0;
			_multiplier = 1.0f;
			_output = 0.0f;
			break;
		default:
			_multiplier = 0.0f;		
			_output = 0.0f;
			_phase = 0;
			break;
		}
	}
	_output *= _multiplier;
	_phase += 1;
	return _output * (_max - _min) + _min;
}

void adenv::calculate_multiplier(float start, float end, uint32_t length_in_samples) 
{
	_multiplier = 1.0f + ((end - start) / (float)length_in_samples);
}
