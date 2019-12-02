// # nlfilt
//
// port by: stephen hensley, December 2019
//
// Non-linear filter. 
//
//
// The four 5-coefficients: a, b, d, C, and L are used to configure different filter types.
//
// Though traditional filter types can be made, 
// the effect will always respond differently to different input.
// 
// Structure for Dobson/Fitch nonlinear filter 
//
// This Source is a heavily modified version of the original
// source from Csound.
//
// TODO: 
// - Fix `process()` to process input, and return output;
// - consider un-pointering all the controls, as it currently works different from all other modules.
// - add setters for necessary parameters.
//

#pragma once
#ifndef DSY_NLFILT_H
#define DSY_NLFILT_H
#include <stdlib.h>
#include <stdint.h>
#define DSY_NLFILT_MAX_DELAY 1024 

namespace daisysp 
{
    class nlfilt
    {
        public:

// ### init
// Initializes the nlfilt object, setting the size of the delay line to be used within.
//
// `size_t size`: size of the internal delay line
//      - range: 1-1024
// ~~~~
        void init(size_t size);
// ~~~~

// ### process
// Process the variable pointed to by *_in and updates the output to *_ar;
// ~~~~
        void process();
// ~~~~

        private:

        int32_t set();
        int32_t nlfilt2();

		float *_ar, *_in, *_a, *_b, *_d, *_C, *_L;
		size_t _size;
		float _delay[DSY_NLFILT_MAX_DELAY];
		int32_t _point;
    };
} // namespace daisysp

#endif
