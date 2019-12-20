#include "hid_ctrl.h"
#include <math.h>
// Temporary amount to prevent bleed on the bottom of the pots/CVs
#define BOTTOM_THRESH 0.002f
using namespace daisy;
float hid_ctrl::process() { 
	float t;
	t = (float)*raw_ / 65536.0f;
	t = (t - offset_) * scale_ * (flip_ ? -1.0f : 1.0f);
	if(t < BOTTOM_THRESH)
		t = 0.0f;
	val_ += coeff_ * (t - val_);
	return val_;
}
