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

// ## Mutators

// ### set_downsample_factor
// Sets amount of downsample 
// Input range: 

// ~~~~
		inline void set_downsample_factor(float downsample_factor) 
// ~~~~
		{
			downsample_factor_ = downsample_factor;
		}

// ### set_bitcrush_factor
// Sets amount of bitcrushing 
// Input range: 

// ~~~~
		inline void set_bitcrush_factor(float bitcrush_factor)
// ~~~~
		{
			//			bitcrush_factor_ = bitcrush_factor;
			bits_to_crush_ = bitcrush_factor * k_max_bits_to_crush;
		}

// ### set_bits_to_crush
// Sets the exact number of bits to crush
// 
// 0-16 bits
// ~~~~
		inline void set_bits_to_crush(const uint8_t &bits) 
// ~~~~
		{
			bits_to_crush_ = bits <= k_max_bits_to_crush ? bits : k_max_bits_to_crush;
		}

// ## Accessors

// ### get_downsample_factor
// Returns current setting of downsample

// ~~~~
		inline float get_downsample_factor() { return downsample_factor_; }
// ~~~~

// ### get_bitcrush_factor
// Returns current setting of bitcrush

// ~~~~
		inline float get_bitcrush_factor() { return bitcrush_factor_; }
// ~~~~

	  private:
		const uint8_t k_max_bits_to_crush = 16;
		float downsample_factor_, bitcrush_factor_;
		uint32_t bits_to_crush_;
		float downsampled_, bitcrushed_;
		uint32_t inc_, threshold_;
	};
} // namespace daisysp
#endif
#endif
