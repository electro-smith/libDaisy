#include "util/color.h"
#include <algorithm>

using namespace daisy;

#define RED 0
#define GREEN 1
#define BLUE 2

namespace daisy
{
template <typename T>
T clamp(T in, T low, T high)
{
    return (in < low) ? low : (high < in) ? high : in;
}
} // namespace daisy

const float Color::standard_colors[Color::LAST][3] = {
    {1.0f, 0.0f, 0.0f},   // RED
    {0.0f, 1.0f, 0.0f},   // GREEN
    {0.0f, 0.0f, 1.0f},   // BLUE
    {1.0f, 1.0f, 1.0f},   // WHITE
    {0.25f, 0.0f, 1.0f},  // PURPLE
    {0.0f, 0.25f, 0.75f}, // CYAN
    {1.0f, 0.33f, 0.0f},  // GOLD
    {0.0f, 0.0f, 0.0f},   // OFF
};

void Color::Init(PresetColor c)
{
    red_   = standard_colors[c][RED];
    green_ = standard_colors[c][GREEN];
    blue_  = standard_colors[c][BLUE];
}

void Color::Init(float red, float green, float blue)
{
    red_   = clamp(red, 0.0f, 1.0f);
    green_ = clamp(green, 0.0f, 1.0f);
    blue_  = clamp(blue, 0.0f, 1.0f);
}
