#include "crossfade.h"

using namespace daisysp;

float crossfade::process(float *in1, float *in2)
{
	switch (_curve)
	{
		case CROSSFADE_LIN: 
    		return ((*in2 * _pos) + *in1) * (1 - _pos);

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
