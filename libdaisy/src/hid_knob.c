#include "hid_knob.h"
#include <math.h>
void dsy_ctrl_init(dsy_ctrl *p,
				   uint16_t *rawptr,
				   float	 scale,
				   float	 offset,
				   float	 slewrate,
				   uint8_t   change_thresh,
				   uint8_t   flip,
				   float	 sr)
{
	p->raw   = rawptr;
	p->sr	= sr;
	// divide slew * sr by 2 in order to get actual slew time.
	p->coeff = 1 / (slewrate * sr * 0.5f);
	p->thresh = change_thresh == DSY_KNOB_THRESH_LOCK ? 0.001f : 0.0f;
	p->flip = flip;
	p->lock = 0;
}

void dsy_ctrl_init_knob(dsy_ctrl *p, uint16_t *rawptr, float sr) 
{
	p->raw   = rawptr;
	p->sr	= sr;
	// 10ms slew for pots
	p->coeff = 1 / (0.04f * sr * 0.5f);
	p->thresh = 0.005f;
	p->flip = 0;
	p->lock = 0;
	p->scale  = 1.0f;
	p->offset = 0.0f;
}

void dsy_ctrl_init_bipolar_cv(dsy_ctrl *p, uint16_t *rawptr, float sr)
{
	p->raw   = rawptr;
	p->sr	= sr;
	// 10ms slew for pots
	p->coeff = 1 / (0.005f * sr * 0.5f);
	p->thresh = 0.045f;
	p->flip = 1;
	p->lock = 0;
	p->scale  = 2.0f;
	p->offset = 0.5f;
}


float dsy_ctrl_process(dsy_ctrl *p) 
{
	float t;
	// normalize to 0-1
	t = (float)*p->raw / 65536.0f;
	// offset, flip, scale
	t -= p->offset;
	t = p->flip ? -t : t;
	t *= p->scale;
	// high pass filter for change detection
	p->delta = (t - p->prev) + (0.9995f * p->delta);
	p->prev  = t;
	// lock if the output is not moving
	p->lock = fabsf(p->delta) < p->thresh ? 1 : 0;
	if(p->lock)
		t = p->val;
	// smoothing low pass filter.
	p->val += p->coeff * (t - p->val);
	return p->val;
}
