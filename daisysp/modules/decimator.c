#include "decimator.h"

#define MAX_BITS_TO_CRUSH 16

void dsy_decimator_init(dsy_decimator_t *d)
{
	d->downsample_factor = 1.0f;
	d->bitcrush_factor = 0.0f;
	d->downsampled = 0.0f;
	d->bitcrushed = 0.0f;
	d->inc = 0;
	d->threshold = 0;
}

float dsy_decimator_process(dsy_decimator_t *d, float input)
{
	uint8_t bits_to_crush;
	int32_t temp;
	//downsample
	d->threshold = (uint32_t)((d->downsample_factor *d->downsample_factor) * 96.0f);
	d->inc += 1;
	if (d->inc > d->threshold)
	{
		d->inc = 0;
		d->downsampled = input;
	}
	//bitcrush
	// there are probably a million different ways to do this better.
	bits_to_crush = (uint8_t)(d->bitcrush_factor * MAX_BITS_TO_CRUSH);
	if (bits_to_crush > MAX_BITS_TO_CRUSH)
	{
		bits_to_crush = MAX_BITS_TO_CRUSH;
	}
	temp = (int32_t)(d->downsampled * 65536.0f);
	temp >>= bits_to_crush;  // shift off
	temp <<= bits_to_crush;  // move back with zeros
	d->bitcrushed = (float)temp / 65536.0f;
	//d->output = d->bitcrushed;
	return d->bitcrushed;
}