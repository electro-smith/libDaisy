// # compressor
// 
// influenced by compressor in soundpipe (from faust).
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

    void init(float samplerate);
    //float process(const float &in);
	float process(float &in, float &key);
	float process(float &in);

	// Expects 0.001 -> 40.
    void set_ratio(const float &ratio)
    {
        ratio_ = ratio;
		recalculate_slopes();
	}

    // Expects 0.0 -> -80.
    void set_threshold(const float &thresh)
    {
        thresh_ = thresh;
		recalculate_slopes();
	}

    // Expects 0.001 -> 10
    void set_attack(const float &atk)
    {
        atk_ = atk;
		recalculate_slopes();
    }
    // Expects 0.001 -> 10
    void set_release(const float &rel)
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
