/*
TODO:
- Add Blend(), Scale(), etc.
- I'd also like to change the way the Color names are accessed.
*/
/**        I'd like for it to be easy and not `Color::PresetColor::Foo`
*/
/** - There's no way to change a color once its been created (without unintuitively reiniting it).
*/
#pragma once
#ifndef DSY_COLOR_H
#define DSY_COLOR_H
#include <stdint.h>


namespace daisy
{
/** @addtogroup utility
    @{
*/

/** Class for handling simple colors */
class Color
{
  public:
    Color() : red_(0.f), green_(0.f), blue_(0.f) {}
    Color(float r, float g, float b) : red_(r), green_(g), blue_(b) {}
    ~Color() {}

    /** List of colors that have a preset RGB value */
    enum PresetColor
    {
        RED,    /**< & */
        GREEN,  /**< & */
        BLUE,   /**< & */
        WHITE,  /**< & */
        PURPLE, /**< & */
        CYAN,   /**< & */
        GOLD,   /**< & */
        OFF,    /**< & */
        LAST    /**< & */
    };

    /** Initializes the Color with a given preset.
    \param c Color to init to
    */
    void Init(PresetColor c);

    /** Initializes the Color with a specific RGB value
    red, green, and blue should be floats between 0 and 1
    \param red Red value
    \param green Green value
    \param blue Blue value
    */
    void Init(float red, float green, float blue);

    /** Returns the 0-1 value for Red */
    inline float Red() const { return red_; }

    /** Returns the 0-1 value for Green */
    inline float Green() const { return green_; }

    /** Returns the 0-1 value for Blue */
    inline float Blue() const { return blue_; }

    inline uint8_t Red8() const { return red_ * 255; }
    inline uint8_t Green8() const { return green_ * 255; }
    inline uint8_t Blue8() const { return blue_ * 255; }

    inline void SetRed(const float amt) { red_ = amt; }
    inline void SetGreen(const float amt) { green_ = amt; }
    inline void SetBlue(const float amt) { blue_ = amt; }

    /** Returns a scaled color by a float */
    Color operator*(float scale)
    {
        Color c;
        c.Init(red_ * scale, green_ * scale, blue_ * scale);
        return c;
    }

    /** Adds another color to this one, destructively saturating at 1 */
    Color operator+(Color rhs)
    {
        float r_ = red_ + rhs.Red();
        float g_ = green_ + rhs.Green();
        float b_ = blue_ + rhs.Blue();
        if(r_ > 1.f)
            r_ = 1.f;
        if(g_ > 1.f)
            g_ = 1.f;
        if(b_ > 1.f)
            b_ = 1.f;
        Color c(r_, g_, b_);
        return c;
    }

    /** Returns a color that is blended between a and b */
    static Color Blend(const Color a, const Color b, const float amt)
    {
        float scalar = amt > 1.f ? 1.f : amt < 0.f ? 0.f : amt;
        float nr     = a.Red() + (b.Red() - a.Red()) * scalar;
        float ng     = a.Green() + (b.Green() - a.Green()) * scalar;
        float nb     = a.Blue() + (b.Blue() - a.Blue()) * scalar;

        Color new_color(nr, ng, nb);
        return new_color;
    }


  private:
    static const float standard_colors[LAST][3];
    float              red_, green_, blue_;
};
/** @} */
} // namespace daisy

#endif
