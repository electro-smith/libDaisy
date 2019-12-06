// # decimator
// Performs downsampling and bitcrush effects

#pragma once
#ifndef DECIMATOR_H
#define DECIMATOR_H
#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{
	class decimator
	{
	public:
		decimator() {}
		~decimator() {}

// ### init
// Initializes downsample module
// ~~~~
		void init();
// ~~~~

// ### process
// Applies downsample and bitcrush effects to input signal.
// Returns one sample. This should be called once per sample period. 
// ~~~~
		float process(float input);
// ~~~~

// ## Setters

// ### set_downsample_factor
// Sets amount of downsample 
// Input range: 

// ~~~~
		inline void set_downsample_factor (float downsample_factor) 
// ~~~~
		{
			_downsample_factor = downsample_factor;
		}

// ### set_bitcrush_factor
// Sets amount of bitcrushing 
// Input range: 

// ~~~~
		inline void set_bitcrush_factor (float bitcrush_factor)
// ~~~~
		{
			//			_bitcrush_factor = bitcrush_factor;
			_bits_to_crush = bitcrush_factor * max_bits_to_crush;
		}

		// ### set_bits_to_crush
		// Sets the exact number of bits to crush
		// 0-16 bits
		inline void set_bits_to_crush(const uint8_t &bits) 
		{
			_bits_to_crush = bits <= max_bits_to_crush ? bits : max_bits_to_crush;
		}

		// ## Getters

		// ### get_downsample_factor
		// Returns current setting of downsample

		// ~~~~
		inline float get_downsample_factor () { return _downsample_factor; }
// ~~~~

// ### get_bitcrush_factor
// Returns current setting of bitcrush

// ~~~~
		inline float get_bitcrush_factor () { return _bitcrush_factor; }
		// ~~~~

	  private:
		const uint8_t max_bits_to_crush = 16;
		float _downsample_factor, _bitcrush_factor;
		uint32_t _bits_to_crush;
		float _downsampled, _bitcrushed;
		uint32_t _inc, _threshold;
	};
} // namespace daisysp
#endif
#endif
