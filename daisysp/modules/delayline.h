// Simple Delay Line
//
// November 2019
//
// Converted to Template December 2019
//
// By: shensley
//
#pragma once
#ifndef DSY_DELAY_H
#define DSY_DELAY_H
#include <stdlib.h>
#include <stdint.h>
namespace daisysp
{
template<typename T, size_t max_size>
class delayline
{
    public:
    delayline() { }
    ~delayline() { }

    void init()
    {
        reset();
    }

    void reset() {
        for (size_t i = 0; i < max_size; i++)
        {
            line_[i] = T(0);
        }
        write_ptr_ = 0;
        delay_ = 1;
    }

    inline void set_delay(size_t delay)
    {
        frac_ = 0.0f;
        delay_ = delay < max_size ? delay : max_size - 1;
    }

    inline void set_delay(float delay)
    {
        int32_t int_delay = static_cast<int32_t>(delay);
        frac_ = delay - static_cast<float>(int_delay);
        delay_ = int_delay < max_size ? int_delay : max_size - 1;
    }

    inline void write(const T sample) 
    {
        line_[write_ptr_] = sample;
        write_ptr_ = (write_ptr_ - 1 + max_size) % max_size;
    }

    inline const T read() const
    {
        T a = line_[(write_ptr_ + delay_) % max_size];
        T b = line_[(write_ptr_ + delay_ + 1) % max_size];
        return a + (b - a) * frac_;
    }

    private:
    float frac_;
    size_t write_ptr_;
    size_t delay_;
    T line_[max_size];

};
} // namespace daisysp
#endif
