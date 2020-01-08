#pragma once
#ifndef DAISY_MODE
#define DAISY_MODE

namespace daisysp
{
class Mode
{
  public:
    Mode() {}
    ~Mode() {}
    void        Init(float sample_rate);
    float       Process(float in);
    void        Clear();
    inline void SetFreq(float freq) { freq_ = freq; }
    inline void SetQ(float q) { q_ = q; }

  private:
    float freq_, q_;
    float xnm1_, ynm1_, ynm2_, a0_, a1_, a2_;
    float d_, lfq_, lq_, sr_;
};
} // namespace daisysp

#endif
