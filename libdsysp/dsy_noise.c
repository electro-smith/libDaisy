#include "dsy_noise.h"
#ifdef __cplusplus
extern "C"
{
#endif
static const float noise_coeff = 4.6566129e-010f;

void dsy_noise_init(dsy_noise_t *p)
{
	p->amp = 1.0f;
	p->randseed = 1;
}
float dsy_noise_process(dsy_noise_t *p)
{
	p->randseed *= 16807;
	return (p->randseed * noise_coeff) * p->amp;
}
#ifdef __cplusplus
}
#endif