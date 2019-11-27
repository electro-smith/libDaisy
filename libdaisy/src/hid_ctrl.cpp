#include "hid_ctrl.h"
#include <math.h>
using namespace daisy;
float hid_ctrl::process() { 
	float t;
	t = (float)*raw / 65536.0f;
	t = (t - offset) * scale * (flip ? -1.0f : 1.0f);
	delta = (t - prev) + (0.999f * delta);
	prev  = t;
	lockstatus = fabsf(delta) < thresh ? true : false;
	if(lockstatus)
		t = val;
	val += coeff * (t - val);
	return val;
}
