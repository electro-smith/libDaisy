#pragma once
#ifndef DSY_DISPLAY_H
#define DSY_DISPLAY_H /**< Macro */
#include <cmath>
#include "util/oled_fonts.h"
#include "daisy_core.h"
#include "graphics_common.h"

#ifndef deg2rad
#define deg2rad(deg) ((deg)*3.141592 / 180.0)
#endif

namespace daisy
{
/** 
 * This interface is used as a base class for all types of 1bit-per-pixel
 * graphics displays.
*/
class OneBitGraphicsDisplay
{
  public:
    OneBitGraphicsDisplay() {}
    virtual ~OneBitGraphicsDisplay() {}

    virtual uint16_t Height() const = 0;
    virtual uint16_t Width() const  = 0;

    Rectangle GetBounds() const
    {
        return Rectangle(int16_t(Width()), int16_t(Height()));
    }


    size_t CurrentX() { return currentX_; };
    size_t CurrentY() { return currentY_; };

    /** 
    Fills the entire display with either on/off.
    \param on Sets on or off.
    */
    virtual void Fill(bool on) = 0;

    /**
    Sets the pixel at the specified coordinate to be on/off.
    \param x   x Coordinate
    \param y   y coordinate
    \param on  on or off
    */
    virtual void DrawPixel(uint_fast8_t x, uint_fast8_t y, bool on) = 0;

    /**
    Draws a line from (x1, y1) to (y1, y2)
    \param x1  x Coordinate of the starting point
    \param y1  y Coordinate of the starting point
    \param x2  x Coordinate of the ending point
    \param y2  y Coordinate of the ending point
    \param on  on or off
    */
    virtual void DrawLine(uint_fast8_t x1,
                          uint_fast8_t y1,
                          uint_fast8_t x2,
                          uint_fast8_t y2,
                          bool         on)
        = 0;

    /**
    Draws a rectangle based on two coordinates.
    \param x1 x Coordinate of the first point
    \param y1 y Coordinate of the first point
    \param x2 x Coordinate of the second point
    \param y2 y Coordinate of the second point
    \param on on or off
    \param fill fill the rectangle or draw only the outline
    */
    virtual void DrawRect(uint_fast8_t x1,
                          uint_fast8_t y1,
                          uint_fast8_t x2,
                          uint_fast8_t y2,
                          bool         on,
                          bool         fill = false)
        = 0;

    /**
    Draws a rectangle.
    \param rect the rectangle
    \param on   on or off
    \param fill fill the rectangle or draw only the outline
    */
    void DrawRect(const Rectangle& rect, bool on, bool fill = false)
    {
        DrawRect(rect.GetX(),
                 rect.GetY(),
                 rect.GetRight(),
                 rect.GetBottom(),
                 on,
                 fill);
    }

    /**
    Draws an arc around the specified coordinate
    \param x           x Coordinate of the center of the arc
    \param y           y Coordinate of the center of the arc
    \param radius      radius of the arc
    \param start_angle angle where to start the arc
    \param sweep       total angle of the arc
    \param on  on or off
    */
    virtual void DrawArc(uint_fast8_t x,
                         uint_fast8_t y,
                         uint_fast8_t radius,
                         int_fast16_t start_angle,
                         int_fast16_t sweep,
                         bool         on)
        = 0;

    /**
    Draws a circle around the specified coordinate
    \param x           x Coordinate of the center of the circle
    \param y           y Coordinate of the center of the circle
    \param radius      radius of the circle
    \param on  on or off
    */
    void
    DrawCircle(uint_fast8_t x, uint_fast8_t y, uint_fast8_t radius, bool on)
    {
        DrawArc(x, y, radius, 0, 360, on);
    };

    /** 
    Writes the character with the specific FontDef
    to the display buffer at the current Cursor position.
    \param ch character to be written
    \param font font to be written in
    \param on    on or off
    \return &
    */
    virtual char WriteChar(char ch, FontDef font, bool on) = 0;

    /** 
    Similar to WriteChar, except it will handle an entire String.
    Wrapping does not happen automatically, so the width
    of the string must be kept within the dimensions of the screen.
    \param str string to be written
    \param font font to use
    \param on  on or off
    \return &
    */
    virtual char WriteString(const char* str, FontDef font, bool on) = 0;

    /** 
    Similar to WriteString but justified within a bounding box.
    \param str          string to be written
    \param font         font to use
    \param boundingBox  the bounding box to draw the text in
    \param alignment    the alignment to use
    \param on           on or off
    \return The rectangle that was drawn to
    */
    virtual Rectangle WriteStringAligned(const char*    str,
                                         const FontDef& font,
                                         Rectangle      boundingBox,
                                         Alignment      alignment,
                                         bool           on)
        = 0;

    /** 
    Moves the 'Cursor' position used for WriteChar, and WriteStr to the specified coordinate.
    \param x x pos
    \param y y pos
    */
    void SetCursor(uint16_t x, uint16_t y)
    {
        currentX_ = (x >= Width()) ? Width() - 1 : x;
        currentY_ = (y >= Height()) ? Height() - 1 : y;
    }

    /** 
    Writes the current display buffer to the OLED device using SPI or I2C depending on 
    how the object was initialized.
    */
    virtual void Update() = 0;

  protected:
    uint16_t currentX_;
    uint16_t currentY_;
};

/** This class is intended as a intermediary class for your actual implementation of the OneBitGraphicsDisplay
 *  interface. It uses the CRTP design pattern where the template argument is the child class. It provides 
 *  implementations for most of the functions, except DrawPixel(), Update() and Fill(), which you'll have
 *  to provide in your child class.
 *  The main goal of this class is to provide common drawing functions without relying on massive amounts of 
 *  virtual function calls that would result in a performance loss. To achieve this, any drawing function that
 *  is implemented here and internally calls other drawing functions (e.g. DrawRect() which internally calls
 *  DrawPixel() and DrawLine()) makes these calls via the qualified name of these functions to explicitly 
 *  suppress the virtual dispatch mechanism like this:
 * 
 *      ChildType::DrawPixel(...); // no virtual function call; direct call into the child class function
 *  
 *  To create a custom OneBitGraphicsDisplay implementation, you can 
 *  A) inherit from OneBitGraphicsDisplay directly and provide all the drawing functions yourself
 *  B) Inherit from OneBitGraphicsDisplayImpl and only provide DrawPixel(), Fill() and Update()
 *     like this:
 *  
 *      class MyDisplayClass : public OneBitGraphicsDisplayImpl<MyDisplayClass> 
 *      {
 *      public:
 *          void Fill() override { ... };
 *          void DrawPixel(uint_fast8_t x, uint_fast8_t y, bool on) override { ... };
 *          void Update() override { ... }
 *      };
 *  
 */
template <class ChildType>
class OneBitGraphicsDisplayImpl : public OneBitGraphicsDisplay
{
  public:
    OneBitGraphicsDisplayImpl() {}
    virtual ~OneBitGraphicsDisplayImpl() {}

    void DrawLine(uint_fast8_t x1,
                  uint_fast8_t y1,
                  uint_fast8_t x2,
                  uint_fast8_t y2,
                  bool         on) override
    {
        int_fast16_t deltaX = abs((int_fast16_t)x2 - (int_fast16_t)x1);
        int_fast16_t deltaY = abs((int_fast16_t)y2 - (int_fast16_t)y1);
        int_fast16_t signX  = ((x1 < x2) ? 1 : -1);
        int_fast16_t signY  = ((y1 < y2) ? 1 : -1);
        int_fast16_t error  = deltaX - deltaY;
        int_fast16_t error2;

        // If we write "ChildType::DrawPixel(x2, y2, on);", we end up with
        // all sorts of weird compiler errors when the Child class is a template
        // class. The only way around this is to use this very verbose syntax:
        ((ChildType*)(this))->ChildType::DrawPixel(x2, y2, on);

        while((x1 != x2) || (y1 != y2))
        {
            ((ChildType*)(this))->ChildType::DrawPixel(x1, y1, on);
            error2 = error * 2;
            if(error2 > -deltaY)
            {
                error -= deltaY;
                x1 += signX;
            }

            if(error2 < deltaX)
            {
                error += deltaX;
                y1 += signY;
            }
        }
    }

    void DrawRect(uint_fast8_t x1,
                  uint_fast8_t y1,
                  uint_fast8_t x2,
                  uint_fast8_t y2,
                  bool         on,
                  bool         fill = false) override
    {
        if(fill)
        {
            for(uint_fast8_t x = x1; x <= x2; x++)
            {
                for(uint_fast8_t y = y1; y <= y2; y++)
                {
                    ((ChildType*)(this))->ChildType::DrawPixel(x, y, on);
                }
            }
        }
        else
        {
            ((ChildType*)(this))->ChildType::DrawLine(x1, y1, x2, y1, on);
            ((ChildType*)(this))->ChildType::DrawLine(x2, y1, x2, y2, on);
            ((ChildType*)(this))->ChildType::DrawLine(x2, y2, x1, y2, on);
            ((ChildType*)(this))->ChildType::DrawLine(x1, y2, x1, y1, on);
        }
    }

    void DrawArc(uint_fast8_t x,
                 uint_fast8_t y,
                 uint_fast8_t radius,
                 int_fast16_t start_angle,
                 int_fast16_t sweep,
                 bool         on) override
    {
        // Values to calculate the circle
        int_fast16_t t_x, t_y, err, e2;

        // Temporary values to speed up comparisons
        float t_sxy, t_syx, t_sxny, t_synx;
        float t_exy, t_eyx, t_exny, t_eynx;

        float start_angle_rad, end_angle_rad;
        float start_x, start_y, end_x, end_y;

        bool d1, d2, d3, d4;

        d1 = d2 = d3 = d4 = true;

        bool circle = false;

        if(sweep < 0)
        {
            start_angle += sweep;
            sweep = -sweep;
        }

        start_angle_rad = deg2rad(start_angle);
        end_angle_rad   = deg2rad(start_angle + sweep);

        start_x = cos(start_angle_rad) * radius;
        start_y = -sin(start_angle_rad) * radius;
        end_x   = cos(end_angle_rad) * radius;
        end_y   = -sin(end_angle_rad) * radius;

        // Check if start and endpoint are very near
        if((end_x - start_x) * (end_x - start_x)
               + (end_y - start_y) * (end_y - start_y)
           < 2.0f)
        {
            if(sweep > 180)
                circle = true;
            else
                // Nothing to draw
                return;
        }

        t_x = -radius;
        t_y = 0;
        err = 2 - 2 * radius;

        do
        {
            if(!circle)
            {
                t_sxy  = start_x * t_y;
                t_syx  = start_y * t_x;
                t_sxny = start_x * -t_y;
                t_synx = start_y * -t_x;
                t_exy  = end_x * t_y;
                t_eyx  = end_y * t_x;
                t_exny = end_x * -t_y;
                t_eynx = end_y * -t_x;

                if(sweep > 180)
                {
                    d1 = (t_sxy - t_synx < 0 || t_exy - t_eynx > 0);
                    d2 = (t_sxy - t_syx < 0 || t_exy - t_eyx > 0);
                    d3 = (t_sxny - t_syx < 0 || t_exny - t_eyx > 0);
                    d4 = (t_sxny - t_synx < 0 || t_exny - t_eynx > 0);
                }
                else
                {
                    d1 = (t_sxy - t_synx < 0 && t_exy - t_eynx > 0);
                    d2 = (t_sxy - t_syx < 0 && t_exy - t_eyx > 0);
                    d3 = (t_sxny - t_syx < 0 && t_exny - t_eyx > 0);
                    d4 = (t_sxny - t_synx < 0 && t_exny - t_eynx > 0);
                }
            }

            if(d1)
                ((ChildType*)(this))
                    ->ChildType::DrawPixel(x - t_x, y + t_y, on);
            if(d2)
                ((ChildType*)(this))
                    ->ChildType::DrawPixel(x + t_x, y + t_y, on);
            if(d3)
                ((ChildType*)(this))
                    ->ChildType::DrawPixel(x + t_x, y - t_y, on);
            if(d4)
                ((ChildType*)(this))
                    ->ChildType::DrawPixel(x - t_x, y - t_y, on);

            e2 = err;
            if(e2 <= t_y)
            {
                t_y++;
                err = err + (t_y * 2 + 1);
                if(-t_x == t_y && e2 <= t_x)
                {
                    e2 = 0;
                }
            }
            if(e2 > t_x)
            {
                t_x++;
                err = err + (t_x * 2 + 1);
            }
        } while(t_x <= 0);
    }

    char WriteChar(char ch, FontDef font, bool on) override
    {
        uint32_t i, b, j;

        // Check if character is valid
        if(ch < 32 || ch > 126)
            return 0;

        // Check remaining space on current line
        if(Width() < (currentX_ + font.FontWidth)
           || Height() < (currentY_ + font.FontHeight))
        {
            // Not enough space on current line
            return 0;
        }

        // Use the font to write
        for(i = 0; i < font.FontHeight; i++)
        {
            b = font.data[(ch - 32) * font.FontHeight + i];
            for(j = 0; j < font.FontWidth; j++)
            {
                if((b << j) & 0x8000)
                {
                    ((ChildType*)(this))
                        ->ChildType::DrawPixel(
                            currentX_ + j, (currentY_ + i), on);
                }
                else
                {
                    ((ChildType*)(this))
                        ->ChildType::DrawPixel(
                            currentX_ + j, (currentY_ + i), !on);
                }
            }
        }

        // The current space is now taken
        SetCursor(currentX_ + font.FontWidth, currentY_);

        // Return written char for validation
        return ch;
    }

    char WriteString(const char* str, FontDef font, bool on) override
    {
        // Write until null-byte
        while(*str)
        {
            if(((ChildType*)(this))->ChildType::WriteChar(*str, font, on)
               != *str)
            {
                // Char could not be written
                return *str;
            }

            // Next char
            str++;
        }

        // Everything ok
        return *str;
    }

    Rectangle WriteStringAligned(const char*    str,
                                 const FontDef& font,
                                 Rectangle      boundingBox,
                                 Alignment      alignment,
                                 bool           on) override
    {
        const auto alignedRect
            = GetTextRect(str, font).AlignedWithin(boundingBox, alignment);
        SetCursor(alignedRect.GetX(), alignedRect.GetY());
        ((ChildType*)(this))->ChildType::WriteString(str, font, on);
        return alignedRect;
    }

  private:
    uint32_t strlen(const char* string)
    {
        uint32_t result = 0;
        while(*string++ != '\0')
            result++;
        return result;
    }

    Rectangle GetTextRect(const char* text, const FontDef& font)
    {
        const auto numChars = strlen(text);
        return {int16_t(numChars * font.FontWidth), font.FontHeight};
    }
};

} // namespace daisy

#endif
