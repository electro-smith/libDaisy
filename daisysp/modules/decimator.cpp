#include "decimator.h"

using namespace daisysp;

#define MAX_BITS_TO_CRUSH 16

void decimator::init()
{
	_downsample_factor = 1.0f;
	_bitcrush_factor = 0.0f;
	_downsampled = 0.0f;
	_bitcrushed = 0.0f;
	_inc = 0;
	_threshold = 0;
}

float decimator::process(float input)
{
	uint8_t bits_to_crush;
	int32_t temp;
	//downsample
	_threshold = (uint32_t)((_downsample_factor *_downsample_factor) * 96.0f);
	_inc += 1;
	if (_inc > _threshold)
	{
		_inc = 0;
		_downsampled = input;
	}
	//bitcrush
	// there are probably a million different ways to do this better.
	bits_to_crush = (uint8_t)(_bitcrush_factor * MAX_BITS_TO_CRUSH);
	if (bits_to_crush > MAX_BITS_TO_CRUSH)
	{
		bits_to_crush = MAX_BITS_TO_CRUSH;
	}
	temp = (int32_t)(_downsampled * 65536.0f);
	temp >>= bits_to_crush;  // shift off
	temp <<= bits_to_crush;  // move back with zeros
	_bitcrushed = (float)temp / 65536.0f;
	//_output = _bitcrushed;
	return _bitcrushed;
}
