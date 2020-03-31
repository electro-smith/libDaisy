#pragma once
#ifndef DSY_H
#define DSY_H
#include <stdint.h>

// TODO:
//
// - Add Blend(), Scale(), etc.
// - I'd also like to change the way the Color names are accessed. 
//		I'd like for it to be easy and not `Color::PresetColor::Foo`
// 

namespace daisy
{
class Color
{
  public:
    Color() {}
    ~Color() {}
    enum PresetColor
    {
        RED,
        GREEN,
        BLUE,
        WHITE,
        PURPLE,
        CYAN,
        GOLD,
        OFF,
        LAST
    };

    void Init(PresetColor c);

    void Init(float red, float green, float blue);

    inline float Red() const { return red_; }
    inline float Green() const { return green_; }
    inline float Blue() const { return blue_; }


  private:
    static const float standard_colors[LAST][3];
    float              red_, green_, blue_;
};
} // namespace daisy

#endif
