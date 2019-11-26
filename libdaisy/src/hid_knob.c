#include "hid_knob.h"
#include <math.h>
void dsy_knob_init(dsy_knob *	  p,
				   uint16_t *	  rawptr,
				   float		   slewrate,
				   dsy_knob_thresh change_thresh,
				   float		   sr)
{
	p->raw   = rawptr;
	p->sr	= sr;
	// divide slew * sr by 2 in order to get actual slew time.
	p->coeff = 1 / (slewrate * sr * 0.5f); 
	switch(change_thresh)
	{
		case DSY_KNOB_THRESH_NO_LOCK: p->thresh = 0.0f; break;
		case DSY_KNOB_THRESH_LOCK: p->thresh = 0.001f; break;
		default: break;
	}
	p->lock = 0;
}

float dsy_knob_process(dsy_knob *p) 
{
	float t;
	t = (float)*p->raw / 65536.0f;
	p->delta = (t - p->prev) + (0.999f * p->delta);
	p->prev  = t;
	p->lock = fabsf(p->delta) < p->thresh ? 1 : 0;
	if(p->lock)
		t = p->val;
	p->val += p->coeff * (t - p->val);
	return p->val;
}
