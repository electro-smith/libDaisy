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
            inline void set_drive(float d) { drive = d; }

            // Getters for all of the outputs.
            inline float low() { return out_low; }
            inline float high() { return out_high; }
            inline float band() { return out_band; }
            inline float notch() { return out_notch; }
            inline float peak() { return out_peak; }

        private:
            float sr, fc, res, drive, freq, damp;
            float pnotch, plow, phigh, pband, ppeak;
            float input;
            float out_low, out_high, out_band, out_peak, out_notch;
        
    };
} // namespace daisysp

#endif
