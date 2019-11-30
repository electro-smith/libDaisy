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
    void init(float samplerate);
    void process(float in1, float in2, float *out1, float *out2);

    inline void set_feedback(float fb) { _feedback = fb; }
    inline void set_lpfreq(float freq) { _lpfreq = freq; }

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

