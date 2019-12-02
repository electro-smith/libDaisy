// # dcblock
// Removes DC component of a signal

#pragma once
#ifndef DCBLOCK_H
#define DCBLOCK_H
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
		void init(float sample_rate);
// ~~~~

// ### process
// performs dcblock process 
// 
// ~~~~
		float process(float in);
// ~~~~

		private:
			float input_, output_, gain_;
	};
}
#endif
#endif
