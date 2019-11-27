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

		void init()
		{
			amp		 = 1.0f;
			randseed = 1;
		}

		inline void set_amp(float a) { amp = a; }

		inline float process() 
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
