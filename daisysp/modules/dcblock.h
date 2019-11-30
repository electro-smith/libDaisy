// # dcblock
// Removes dc component of a signal

#pragma once
#ifndef DCBLOCK_H
#define DCBLOCK_H
#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{
	class dcblock
	{
	public: 
		dcblock() {};
		~dcblock() {};

// ### init
// Initializes dcblock module
// 
// ~~~~
		void init(int sample_rate);
// ~~~~

// ### process
// performs dcblock process 
// 
// ~~~~
		float process(float in);
// ~~~~

		private:
			float _inputs, _outputs, _gain;
	};
}
#endif
#endif
