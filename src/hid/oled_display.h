#pragma once
#ifndef DSY_OLED_DISPLAY_H
#define DSY_OLED_DISPLAY_H /**< Macro */
#include <cmath>
#include "util/oled_fonts.h"
#include "daisy_core.h"

#ifndef deg2rad
#define deg2rad(deg) ((deg)*3.141592 / 180.0)
#endif

namespace daisy
{
/** 
 * This class is for drawing to a monochrome OLED display. 
*/
template <typename DisplayDriver>
class OledDisplay
{
  public:
    OledDisplay() {}
    ~OledDisplay() {}

    struct Config
    {
        typename DisplayDriver::Config driver_config;
    };

    void Init(Config config) { driver_.Init(config.driver_config); }

    uint16_t Height() { return driver_.Height(); }
    uint16_t Width() { return driver_.Width(); }
    uint16_t CurrentX() { return driver_.CurrentX(); };
    uint16_t CurrentY() { return driver_.CurrentY(); };

    /** 
    Fills the entire display with either on/off.
    \param on Sets on or off.
    */
    void Fill(bool on) { driver_.Fill(on); }

    /**
    Sets the pixel at the specified coordinate to be on/off.
    \param x   x Coordinate
    \param y   y coordinate
    \param on  on or off
    */
    void DrawPixel(uint_fast8_t x, uint_fast8_t y, bool on)
    {
        driver_.DrawPixel(x, y, on);
    }

    /**
    Draws a line from (x1, y1) to (y1, y2)
    \param x1  x Coordinate of the starting point
    \param y1  y Coordinate of the starting point
    \param x2  x Coordinate of the ending point
    \param y2  y Coordinate of the ending point
    \param on  on or off
    */
    void DrawLine(uint_fast8_t x1,
                  uint_fast8_t y1,
                  uint_fast8_t x2,
                  uint_fast8_t y2,
                  bool         on)
    {
        int_fast16_t deltaX = abs((int_fast16_t)x2 - (int_fast16_t)x1);
        int_fast16_t deltaY = abs((int_fast16_t)y2 - (int_fast16_t)y1);
        int_fast16_t signX  = ((x1 < x2) ? 1 : -1);
        int_fast16_t signY  = ((y1 < y2) ? 1 : -1);
        int_fast16_t error  = deltaX - deltaY;
        int_fast16_t error2;

        DrawPixel(x2, y2, on);
        while((x1 != x2) || (y1 != y2))
        {
            DrawPixel(x1, y1, on);
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

    /**
    Draws a rectangle based on two coordinates.
    \param x1 x Coordinate of the first point
    \param y1 y Coordinate of the first point
    \param x2 x Coordinate of the second point
    \param y2 y Coordinate of the second point
    \param on on or off
    */
    void DrawRect(uint_fast8_t x1,
                  uint_fast8_t y1,
                  uint_fast8_t x2,
                  uint_fast8_t y2,
                  bool         on,
                  bool         fill = false)
    {
        if(fill)
        {
            for(uint_fast8_t x = x1; x <= x2; x++)
            {
                for(uint_fast8_t y = y1; y <= y2; y++)
                {
                    DrawPixel(x, y, on);
                }
            }
        }
        else
        {
            DrawLine(x1, y1, x2, y1, on);
            DrawLine(x2, y1, x2, y2, on);
            DrawLine(x2, y2, x1, y2, on);
            DrawLine(x1, y2, x1, y1, on);
        }
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
    void DrawArc(uint_fast8_t x,
                 uint_fast8_t y,
                 uint_fast8_t radius,
                 int_fast16_t start_angle,
                 int_fast16_t sweep,
                 bool         on)
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
                DrawPixel(x - t_x, y + t_y, on);
            if(d2)
                DrawPixel(x + t_x, y + t_y, on);
            if(d3)
                DrawPixel(x + t_x, y - t_y, on);
            if(d4)
                DrawPixel(x - t_x, y - t_y, on);

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
    char WriteChar(char ch, FontDef font, bool on)
    {
        uint32_t i, b, j;

        // Check if character is valid
        if(ch < 32 || ch > 126)
            return 0;

        // Check remaining space on current line
        if(driver_.Width() < (driver_.CurrentX() + font.FontWidth)
           || driver_.Height() < (driver_.CurrentY() + font.FontHeight))
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
                    DrawPixel(
                        driver_.CurrentX() + j, (driver_.CurrentY() + i), on);
                }
                else
                {
                    DrawPixel(
                        driver_.CurrentX() + j, (driver_.CurrentY() + i), !on);
                }
            }
        }

        // The current space is now taken
        driver_.SetCursor(driver_.CurrentX() + font.FontWidth,
                          driver_.CurrentY());

        // Return written char for validation
        return ch;
    }
    /** 
    Similar to WriteChar, except it will handle an entire String.
    Wrapping does not happen automatically, so the width
    of the string must be kept within the dimensions of the screen.
    \param str string to be written
    \param font font to use
    \param on  on or off
    \return &
    */
    char WriteString(const char* str, FontDef font, bool on)
    {
        // Write until null-byte
        while(*str)
        {
            if(WriteChar(*str, font, on) != *str)
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

    /** 
    Moves the 'Cursor' position used for WriteChar, and WriteStr to the specified coordinate.
    \param x x pos
    \param y y pos
    */
    void SetCursor(uint8_t x, uint8_t y) { driver_.SetCursor(x, y); }

    /** 
    Writes the current display buffer to the OLED device using SPI or I2C depending on 
    how the object was initialized.
    */
    void Update() { driver_.Update(); }

  private:
    DisplayDriver driver_;

    void Reset() { driver_.Reset(); };
    void SendCommand(uint8_t cmd) { driver_.SendCommand(cmd); };
    void SendData(uint8_t* buff, size_t size) { driver_.SendData(buff, size); };
};
/** @} */

} // namespace daisy

#endif
