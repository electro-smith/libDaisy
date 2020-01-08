#pragma once
#ifndef LIMITER_H
#define LIMITER_H
#include <stdlib.h>
namespace daisysp 
{
class Limiter
{
  public:
    Limiter() {}
    ~Limiter() {}
    void Init();
    void ProcessBlock(float *in, size_t size, float pre_gain);
  private:
    float peak_;
};
} // namespace daisysp
#endif
