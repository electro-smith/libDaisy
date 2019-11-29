// State Variable Filter:
// Double Sampled, Stable
// 
// Credit to Andrew Simper from musicdsp.org
// This is his "State Variable Filter (Double Sampled, Stable)"
// Additional thanks to Laurent de Soras for stability limit, and 
// Stefan Diedrichsen for the correct notch output
//
// Ported by: Stephen Hensley
//
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

            void init(float samplerate);
            void process(float in);

            void set_freq(float f);
            void set_res(float r);
            inline void set_drive(float d) { _drive = d; }

            // Getters for all of the outputs.
            inline float low() { return _out_low; }
            inline float high() { return _out_high; }
            inline float band() { return _out_band; }
            inline float notch() { return _out_notch; }
            inline float peak() { return _out_peak; }

        private:
            float _sr, _fc, _res, _drive, _freq, _damp;
            float _notch, _low, _high, _band, _peak;
            float _input;
            float _out_low, _out_high, _out_band, _out_peak, _out_notch;
        
    };
} // namespace daisysp

#endif
