#include <math.h>
#include "crossfade.h"

using namespace daisysp;

float crossfade::process(float &in1, float &in2)
{
    float scalar_1, scalar_2;
	switch (curve_)
	{
		case CROSSFADE_LIN: 
            scalar_1 = pos_;
    		return (*in1 * (1.0f - scalar_1)) + (*in2 * scalar_1);

    	case CROSSFADE_CPOW:
            scalar_1 = sinf(pos_ * (M_PI * 0.5f));
            scalar_2 = sinf((1.0f - pos_) * (M_PI * 0.5f));
            return (in1 * scalar_2) + (in2 * scalar_1);

    	case CROSSFADE_LOG:
            scalar_1 = expf(pos_ * (logf(1.0) - logf(REALLYSMALLFLOAT)) + logf(REALLYSMALLFLOAT));
    		return (*in1 * (1.0f - scalar_1)) + (*in2 * scalar_1);

    	case CROSSFADE_EXP:
            scalar_1 = pos_ * pos_;
    		return (*in1 * (1.0f - scalar_1)) + (*in2 * scalar_1);

    	default :
    		return 0;
	}
}
