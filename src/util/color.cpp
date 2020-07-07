#include "util/color.h"
using namespace daisy;

#define RED 0
#define GREEN 1
#define BLUE 2

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
    red_   = red;
    green_ = green;
    blue_  = blue;
}
