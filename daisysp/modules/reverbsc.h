// # reverbsc
// 
//     Stereo Reverb
//
// Ported from soundpipe
//
// example:
//
// daisysp/modules/examples/ex_reverbsc
//
#pragma once
#ifndef DSYSP_REVERBSC_H
#define DSYSP_REVERBSC_H

#define DSY_REVERBSC_MAX_SIZE 32767

namespace daisysp
{

typedef struct
{
	int     writePos;
	int     bufferSize;
	int     readPos;
	int     readPosFrac;
	int     readPosFrac_inc;
	int     dummy;
	int     seedVal;
	int     randLine_cnt;
	float filterState;
	float *buf;
}reverbsc_dl;

class reverbsc
{
    public:
        reverbsc() {}
        ~reverbsc() {}
// ### init
// Initializes the reverb module, and sets the samplerate at which the process function will be called.
// ~~~~
    void init(float samplerate);
// ~~~~
    
// ### process
// process the input through the reverb, and updates values of out1, and out2 with the new processed signal.
// ~~~~
    void process(float in1, float in2, float *out1, float *out2);
// ~~~~

// ### set_feedabck
// controls the reverb time. reverb tail becomes infinite when set to 1.0
// 
// range: 0.0 to 1.0
//
// ~~~~
    inline void set_feedback(float fb) { _feedback = fb; }
// ~~~~
// ### set_lpfreq
// controls the internal dampening filter's cutoff frequency.
// 
// range: 0.0 to samplerate / 2
//
// ~~~~
    inline void set_lpfreq(float freq) { _lpfreq = freq; }
// ~~~~

    private:
        void next_random_lineseg(reverbsc_dl *lp, int n);
        int init_delay_line(reverbsc_dl *lp, int n);
        float _feedback, _lpfreq;
        float _iSampleRate, _iPitchMod, _iSkipInit;
        float _sampleRate;
        float _dampFact;
        float _prv_LPFreq;
        int _initDone;
        reverbsc_dl _delayLines[8];
        float _aux[DSY_REVERBSC_MAX_SIZE];
};


} // namespace daisysp
#endif

