#pragma once
#include <stdint.h>
#include <math.h>
#include "hid_ctrl.h"
// # Parameter
// 
//      Simple parameter mapping tool that takes a 0-1 input from an hid_ctrl.
//
// TODO: Move init and process to .cpp file
// - i was cool with them being in the h file until math.h got involved for the log stuff.
namespace daisy
{
// ### curve settings
// Curves are applied to the output signal
// ~~~~
	enum
	{
		PARAM_CURVE_LINEAR,
		PARAM_CURVE_EXP,
		PARAM_CURVE_LOG,
		PARAM_CURVE_CUBE,
		PARAM_CURVE_LAST,
	};
// ~~~~

// ## parameter class
	class parameter
	{
	  public:
		parameter() {}
		~parameter() {}

// ### init
// initialize a parameter using an hid_ctrl object.
//
// hid_ctrl input - object containing the direct link to a hardware control source.
//
// min - bottom of range. (when input is 0.0)
//
// max - top of range (when input is 1.0)
//
// curve - the scaling curve for the input->output transformation.
// ~~~~
		inline void init(hid_ctrl input, float min, float max, uint8_t curve) 
// ~~~~
		{ 
			pmin = min;
			pmax = max;
			pcurve = curve;
			in	 = input;
			lmin   = logf(min < 0.0000001f ? 0.0000001f : min);
			lmax   = logf(max);
		}

// ### process
// processes the input signal, this should be called at the samplerate of the hid_ctrl passed in.
//
// returns a float with the specified transformation applied.
// ~~~~
		inline float process()
// ~~~~
		{ 
			switch(pcurve)
			{
				case PARAM_CURVE_LINEAR: 
					val = (in.process() * (pmax - pmin)) + pmin; 
					break;
				case PARAM_CURVE_EXP:
					val = in.process();
					val = ((val * val) * (pmax - pmin)) + pmin;
					break;
				case PARAM_CURVE_LOG: 
					val = expf((in.process() * (lmax - lmin)) + lmin);
					break;
				case PARAM_CURVE_CUBE:
					val = in.process();
					val = ((val *(val * val)) * (pmax - pmin)) + pmin;
					break;
				default: break;
			}
			return val;
		}

// ### value
// returns the current value from the parameter without processing another sample.
// this is useful if you need to use the value multiple times, and don't store 
// the output of process in a local variable.
// ~~~~
		inline float value() { return val; }
// ~~~~

	  private:
		hid_ctrl in;
		float   pmin, pmax;
		float	lmin, lmax; // for log range
		float	val;
		uint8_t pcurve;
	};
} // namespace daisy
