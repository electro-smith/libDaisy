// # compressor
// 
// influenced by compressor in soundpipe (from faust).
// 
// Modifications made to do:
// 
// - Less calculations during each process loop (coefficients recalculated on parameter change).
// - C++-ified
// - added sidechain support
//
// TODO:
// 
// - With fixed controls this is relatively quick, but changing controls now costs a lot more
// - Still pretty expensive
// - Add soft/hard knee settings 
// - Maybe make stereo possible? (needing two for stereo is a bit silly, 
// and their gain shouldn't be totally unique.
// 
// 
// by: shensley
//
#pragma once
#ifndef DSY_COMPRESSOR_H
#define DSY_COMPRESSOR_H

namespace daisysp
{
class compressor
{
    public:
    compressor() {}
    ~compressor() {}

// ### init
//
// Initializes compressor
// 
// samplerate - rate at which samples will be produced by the audio engine.
// ~~~~
    void init(float samplerate);
// ~~~~

// ### process
// compresses the audio input signal, either keyed by itself, or a secondary input.
// 
// in - audio input signal (to be compressed)
// 
// (optional) key - audio input that will be used to side-chain the compressor. 
// ~~~~
	float process(float &in, float &key);
	float process(float &in);
// ~~~~

// ## setters

// ### set_ratio
// amount of gain reduction applied to compressed signals
// 
// Expects 1.0 -> 40. (untested with values < 1.0)
// ~~~~
    void set_ratio(const float &ratio)
// ~~~~
    {
        ratio_ = ratio;
		recalculate_slopes();
	}

// ### set_threshold
// threshold in dB at which compression will be applied
// 
// Expects 0.0 -> -80.
// ~~~~
    void set_threshold(const float &thresh)
// ~~~~
    {
        thresh_ = thresh;
		recalculate_slopes();
	}

// ### set_attack
// envelope time for onset of compression for signals above the threshold.
// 
// Expects 0.001 -> 10
// ~~~~
    void set_attack(const float &atk)
// ~~~~
    {
        atk_ = atk;
		recalculate_slopes();
    }

// ### set_release
// envelope time for release of compression as input signal falls below threshold.
// 
// Expects 0.001 -> 10
// ~~~~
    void set_release(const float &rel)
// ~~~~
    {
        rel_ = rel;
		recalculate_slopes();
    }

    private:
	  void  recalculate_slopes();
	  float sr_;
	  float ratio_, thresh_, atk_, rel_;
	  float makeupgain_;
	  // internals from faust struct
	  float fRec2_[2], fRec1_[2], fRec0_[2];
	  float fSlow0_, fSlow1_, fSlow2_, fSlow3_, fSlow4_, fSlow5_;
	  int   iConst0_;
	  float fConst1_, fConst2_;
};

} // namespace daisysp

#endif
