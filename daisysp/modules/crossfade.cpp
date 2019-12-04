#include <math.h>
#include "crossfade.h"

#define REALLYSMALLFLOAT 0.000001f

using namespace daisysp;

const float cross_log_min = logf(REALLYSMALLFLOAT);
const float cross_log_max = logf(1.0f);

float crossfade::process(float &in1, float &in2)
{
    float scalar_1, scalar_2;
	switch (curve_)
	{
		case CROSSFADE_LIN: 
            scalar_1 = pos_;
    		return (in1 * (1.0f - scalar_1)) + (in2 * scalar_1);

    	case CROSSFADE_CPOW:
            scalar_1 = sinf(pos_ * (M_PI * 0.5f));
            scalar_2 = sinf((1.0f - pos_) * (M_PI * 0.5f));
            return (in1 * scalar_2) + (in2 * scalar_1);

    	case CROSSFADE_LOG:
            scalar_1 = expf(pos_ * (cross_log_max - cross_log_min) + cross_log_min);
    		return (in1 * (1.0f - scalar_1)) + (in2 * scalar_1);

    	case CROSSFADE_EXP:
            scalar_1 = pos_ * pos_;
    		return (in1 * (1.0f - scalar_1)) + (in2 * scalar_1);

    	default :
    		return 0;
	}
}
