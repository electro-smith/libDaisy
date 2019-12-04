// tone
//
// filter
// 
#pragma once
#ifndef DSY_TONE_H
#define DSY_TONE_H

#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{
	class tone
	{
	public:
		tone() {}
		~tone() {}

		void init(float sample_rate);
		float process(float *in);

		// setters
		set_freq(float freq);

		// getters
		inline void get_freq() { return freq_; }

	private:
		float out_, prevout_, in_, freq_, c1_, c2_, sample_rate_;
		void calculate_coefficients();
/*
typedef struct {
    float out;
    float prevout;
    float in;
    float freq;
    float c1, c2;
}tone;

void tone_init(tone *d);
void tone_set_freq(tone *d, float freq);
void tone_render(tone *d, float *in, float *out);
*/
	};
} // namespace daisysp
#endif
#endif
