// # line
// creates a line segment signal

#pragma once
#ifndef LINE_H
#define LINE_H
#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{
	class line
	{
	public:
		line() {}
		~line() {}
// ### init
// Initializes line module.
// ~~~~
		void init(float sample_rate);
// ~~~~

// ### process
// Processes line segment. Returns one sample.
//
// value of finished will be updated to a 1, upon completion of the line's trajectory.
// ~~~~
		float process(uint8_t *finished);
// ~~~~

// ### start
// Begin creation of line. 
//
// Arguments:
//
// - start - beginning value
// - end - ending value
// - dur - duration in seconds of line segment

// ~~~~
		void start(float start, float end, float dur);
// ~~~~

	private:
		float start_, end_, dur_;
		float inc_, val_, sample_rate_;
		uint8_t finished_;
	};
} // namespace daisysp
#endif
#endif
