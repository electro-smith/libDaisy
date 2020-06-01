// # Color
// ## Description
// Class for handling simple cololrs
//
// ## TODO:
//
// - Add Blend(), Scale(), etc.
// - I'd also like to change the way the Color names are accessed.
//		I'd like for it to be easy and not `Color::PresetColor::Foo`
// - There's no way to change a color once its been created (without unintuitively reiniting it).
//
#pragma once
#ifndef DSY_COLOR_H
#define DSY_COLOR_H
#include <stdint.h>

namespace daisy
{
class Color
{
  public:
    Color() {}
    ~Color() {}
	// ### PresetColor
	// List of colors that have a preset RGB value
	// 
	// ~~~~
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
    // ~~~~

	// ### Init
	// Initializes the Color with a given preset.
	// ~~~~
    void Init(PresetColor c);
    // ~~~~

	// Initializes the Color with a specific RGB value
	// 
    // red, green, and blue should be floats between 0 and 1
	// ~~~~
    void Init(float red, float green, float blue);
	// ~~~~

	// ### Accessors
	// Returns the 0-1 value for the given color
	// ~~~~
    inline float Red() const { return red_; }
    inline float Green() const { return green_; }
    inline float Blue() const { return blue_; }
	// ~~~~

  private:
    static const float standard_colors[LAST][3];
    float              red_, green_, blue_;
};
} // namespace daisy

#endif
