// # whitenoise
// 
// fast white noise generator
// I think this came from musicdsp.org at some point
//
#pragma once
#ifndef DSY_WHITENOISE_H
#define DSY_WHITENOISE_H
#include <stdint.h>
#ifdef __cplusplus
namespace daisysp
{
	class whitenoise
	{
	  public:
		whitenoise() {}
		~whitenoise() {}

// ### init
// Initializes the whitenoise object
// ~~~~
		void init()
// ~~~~
		{
			amp		 = 1.0f;
			randseed = 1;
		}

// ### set_amp
// sets the amplitude of the noise output
// ~~~~
		inline void set_amp(float a) { amp = a; }
// ~~~~

// ### process
// returns a new sample of noise in the range of -amp to amp
// ~~~~
		inline float process() 
// ~~~~
		{ 
			randseed *= 16807;
			return (randseed * coeff) * amp;
		}

	  private:
		static constexpr float coeff = 4.6566129e-010f;
		float   amp;
		int32_t randseed;
	};
} // namespace daisysp
#endif
#endif
