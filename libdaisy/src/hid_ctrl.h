//#pragma once
#ifndef DSY_KNOB_H
#define DSY_KNOB_H
#include <stdint.h>

#ifdef __cplusplus
namespace daisy
{
	class hid_ctrl
	{
	  public:
		hid_ctrl() {}
		~hid_ctrl() {}

		void init(uint16_t *adcptr, float sr) 
		{ 
			delta = val = prev = 0.0f;
			raw				   = adcptr;
			samplerate		   = sr;
			coeff			   = 1.0f / (0.04f * samplerate * 0.5f);
			thresh			   = 0.005f;
			scale			   = 1.0f;
			offset			   = 0.0f;
			lockstatus		   = false;
			flip			   = false;
		}

		float process();


	  private:
		uint16_t *raw;
		float	 coeff, thresh, samplerate, val, prev, delta;
		float	 scale, offset;
		bool	  lockstatus, flip;
	};
} // namespace daisy
#endif
#endif
