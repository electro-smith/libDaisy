#pragma once
#include <stdint.h>
#include <math.h>
// TODO: Move init and process to .cpp file
// - i was cool with them being in the h file until math.h got involved for the log stuff.
// TODO: add some sort of check to see if class T has a process() function
namespace daisysp
{
	template<class T>
	class parameter
	{
	  public:
		enum
		{
			CURVE_LINEAR,
			CURVE_EXP,
			CURVE_LOG,
			CURVE_CUBE,
			CURVE_LAST,
		};
		parameter() {}
		~parameter() {}

		inline void init(T input, float min, float max, float curve) 
		{ 
			pmin = min;
			pmax = max;
			pcurve = curve;
			in	 = input;
			// prevent NaN/-Infinity
			lmin   = logf(min < 0.0000001f ? 0.0000001f : min);
			lmax   = logf(max);
		}

		inline float process()
		{ 
			float t;
			switch(pcurve)
			{
				case CURVE_LINEAR: 
					t = (in.process() * (pmax - pmin)) + pmin; 
					break;
				case CURVE_EXP:
					t = in.process();
					t = ((t * t) * (pmax - pmin)) + pmin;
					break;
				case CURVE_LOG: 
					t = expf((in.process() * (lmax - lmin)) + lmin);
					break;
				case CURVE_CUBE:
					t = in.process();
					t = ((t *(t * t)) * (pmax - pmin)) + pmin;
					break;
				default: break;
			}
			return t;
		}

	  private:
		T in;
		float   pmin, pmax;
		float   lmin, lmax; // for log range
		uint8_t pcurve;
	};
} // namespace daisy
