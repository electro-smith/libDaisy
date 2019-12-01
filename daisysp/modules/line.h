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
// Expected input: 
// ~~~~
		float process(uint8_t *finished);
// ~~~~

// ### start
// Begin creation of line. 
// Arguments:
// - start - beginning value
// - end - ending value
// - dur - duration in seconds of line segment

// ~~~~
		void start(float start, float end, float dur);
// ~~~~

	private:
		float _start, _end, _dur;
		float _inc, _val, _sr;
		uint8_t _finished;
	};
} // namespace daisysp
#endif
#endif
