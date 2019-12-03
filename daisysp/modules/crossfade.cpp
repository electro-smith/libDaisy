#include <math.h>
#include "crossfade.h"

using namespace daisysp;

float crossfade::process(float *in1, float *in2)
{
    // const pow scalar
    float scalar = sinf(pos_ * (M_PI * 0.5f));
	switch (curve_)
	{
		case CROSSFADE_LIN: 
    		return ((*in2 * pos_) + *in1) * (1 - pos_);

    	case CROSSFADE_CPOW:
            return (*in1 * (1.0f - scalar)) + (*in2 * scalar);

    	case CROSSFADE_LOG:
    		return 0;

    	case CROSSFADE_EXP:
    		return 0;

    	default :
    		return 0;
	}
}
