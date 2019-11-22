#include "whitenoise.h"

static const float noise_coeff = 4.6566129e-010f;

void dsy_whitenoise_init(dsy_whitenoise *p)
{
	p->amp = 1.0f;
	p->randseed = 1;
}

float dsy_whitenoise_process(dsy_whitenoise *p)
{
	p->randseed *= 16807;
	return (p->randseed * noise_coeff) * p->amp;
}
