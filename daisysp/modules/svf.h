// # svf
//      Double Sampled, Stable State Variable Filter
// 
// Credit to Andrew Simper from musicdsp.org
//
// This is his "State Variable Filter (Double Sampled, Stable)"
//
// Additional thanks to Laurent de Soras for stability limit, and 
// Stefan Diedrichsen for the correct notch output
//
// Ported by: Stephen Hensley
//
// example:
// daisysp/examples/svf/
#pragma once
#ifndef DSY_SVF_H
#define DSY_SVF_H

namespace daisysp
{
    class svf
    {
        public:
            svf() {}
            ~svf() {}

// ### init
// 
// Initializes the filter
// 
// float samplerate - sample rate of the audio engine being run, and the frequency that the process function will be called.
// ~~~~
            void init(float samplerate);
// ~~~~


// ### process
// 
// Process the input signal, updating all of the outputs.
// ~~~~
            void process(float in);
// ~~~~

// ## Setters
//
// ### set_freq
//
// sets the frequency of the cutoff frequency. 
// 
// f must be between 0.0 and samplerate / 2
// ~~~~
            void set_freq(float f);
// ~~~~

// ### set_res
// 
// sets the resonance of the filter.
//
// Must be between 0.0 and 1.0 to ensure stability.
// ~~~~
            void set_res(float r);
// ~~~~

// ### set_drive
// 
// sets the drive of the filter, affecting the response of the resonance of
// the filter..
// ~~~~
            inline void set_drive(float d) { _drive = d; }
// ~~~~

// ## Filter Outputs
// ## Lowpass Filter
// ~~~~
            inline float low() { return _out_low; }
// ~~~~
// ## Highpass Filter
// ~~~~
            inline float high() { return _out_high; }
// ~~~~
// ## Bandpass Filter
// ~~~~
            inline float band() { return _out_band; }
// ~~~~
// ## Notch Filter
// ~~~~
            inline float notch() { return _out_notch; }
// ~~~~
// ## Peak Filter
// ~~~~
            inline float peak() { return _out_peak; }
// ~~~~

        private:
            float _sr, _fc, _res, _drive, _freq, _damp;
            float _notch, _low, _high, _band, _peak;
            float _input;
            float _out_low, _out_high, _out_band, _out_peak, _out_notch;
        
    };
} // namespace daisysp

#endif
