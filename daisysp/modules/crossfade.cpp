#include "crossfade.h"

using namespace daisysp;

float crossfade::process(float *in1, float *in2)
{
	switch (curve_)
	{
		case CROSSFADE_LIN: 
    		return ((*in2 * pos_) + *in1) * (1 - pos_);

    	case CROSSFADE_CPOW:
    		return 0;

    	case CROSSFADE_LOG:
    		return 0;

    	case CROSSFADE_EXP:
    		return 0;

    	default :
    		return 0;
	}
}
