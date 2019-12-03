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
namespace daisysp
{
template<typename T, size_t max_size>
class delayline
{
    public:
    DelayLine() { }
    ~DelayLine() { }

    void init()
    {
        reset();
    }

    void reset() {
        for (size_t i = 0; i < max_size; i++)
        {
            line[i] = T(0);
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
        delay_ = int_delay < max_size ? int_int_delay : max_size - 1;
    }

    inline void write(const T sample) 
    {
        line_[write_ptr_] = sample;
        write_ptr_ = (write_ptr_ - 1 + max_delay) % max_delay;
    }

    inline const T read() const
    {
        T a = line_[(write_ptr_ + delay_) % max_delay];
        T b = line_[(write_ptr_ + delay_ + 1) % max_delay];
        return a + (b - a) * frac_;
    }

    private:
    float frac_;
    size_t write_ptr_;
    size_t delay_;
    T line[max_size];

};
} // namespace daisysp
#endif
