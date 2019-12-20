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

// ## parameter class
class parameter
{
  public:
// ## Data Types
// ### Curve 
// Curves are applied to the output signal
// ~~~~
	enum Curve
	{
		LINEAR,
		EXP,
		LOG,
		CUBE,
		LAST,
	};
// ~~~~
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
	inline void init(hid_ctrl input, float min, float max, Curve curve) 
// ~~~~
	{ 
		pmin_ = min;
		pmax_ = max;
		pcurve_ = curve;
		in_	 = input;
		lmin_   = logf(min < 0.0000001f ? 0.0000001f : min);
		lmax_   = logf(max);
	}

// ### process
// processes the input signal, this should be called at the samplerate of the hid_ctrl passed in.
//
// returns a float with the specified transformation applied.
// ~~~~
	inline float process()
// ~~~~
	{ 
		switch(pcurve_)
		{
			case LINEAR: 
				val_ = (in_.process() * (pmax_ - pmin_)) + pmin_; 
				break;
			case EXP:
				val_ = in_.process();
				val_ = ((val_ * val_) * (pmax_ - pmin_)) + pmin_;
				break;
			case LOG: 
				val_ = expf((in_.process() * (lmax_ - lmin_)) + lmin_);
				break;
			case CUBE:
				val_ = in_.process();
				val_ = ((val_ *(val_ * val_)) * (pmax_ - pmin_)) + pmin_;
				break;
			default: break;
		}
		return val_;
	}

// ### value
// returns the current value from the parameter without processing another sample.
// this is useful if you need to use the value multiple times, and don't store 
// the output of process in a local variable.
// ~~~~
	inline float value() { return val_; }
// ~~~~

  private:
	hid_ctrl in_;
	float   pmin_, pmax_;
	float	lmin_, lmax_; // for log range
	float	val_;
	Curve pcurve_;
};
} // namespace daisy
