
// Crossfade
// Performs a crossfade between two signals
// 
// Original author: Paul Batchelor
//
// Ported from Soundpipe by Andrew Ikenberry
// added curve option for constant power, etc.
// ToDO: implement all non linear curve options in process()

#pragma once
#ifndef CROSSFADE_H
#define CROSSFADE_H
#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{
	enum 
	{
		CROSSFADE_LIN,
		CROSSFADE_CPOW,
		CROSSFADE_LOG,
		CROSSFADE_EXP,
		CROSSFADE_LAST,
	};

	class crossfade
	{
	public:
		crossfade() {}
		~crossfade() {}

		// init and process functions
		inline void crossfade_init() 
		{
			_pos = 0.5;
    		_curve = CROSSFADE_LIN;
		}
		float process(float *in1, float *in2);

		// setters
		inline void set_pos(float pos) { _pos = pos; }
		inline void set_curve(uint8_t curve) { _curve = curve; }

		// getters
		inline float get_pos(float pos) { return _pos; }
		inline uint8_t get_curve(uint8_t curve) { return _curve; }

	// private variables
	private:
		float _pos;
		uint8_t _curve;


		/*

		typedef struct
		{
			float pos;
			uint8_t curve;
		} crossfade;

		// initialization
		void crossfade_init(crossfade *p, uint8_t curve);

		// processing
		float crossfade_process(crossfade *p, float *in1, float *in2);

		// set position between two signals. range: 0-1
		void crossfade_set_pos(crossfade *p, float pos);

		// set curve of crossfade.
		void crossfade_set_curve(crossfade *p, uint8_t curve);

		*/
	};
} // namespace daisysp
#endif
#endif