#pragma once
#ifndef DSY_OLED_DISPLAY_H
#define DSY_OLED_DISPLAY_H /**< Macro */

#include "display.h"

namespace daisy
{
/** 
 * This class is for drawing to a monochrome OLED display. 
 * @ingroup device
*/
template <typename DisplayDriver>
class OledDisplay : public OneBitGraphicsDisplayImpl<OledDisplay<DisplayDriver>>
{
  public:
    OledDisplay() {}
    virtual ~OledDisplay() {}

    struct Config
    {
        typename DisplayDriver::Config driver_config;
    };

    void Init(Config config) { driver_.Init(config.driver_config); }

    uint16_t Height() const override { return driver_.Height(); }
    uint16_t Width() const override { return driver_.Width(); }

    /** 
    Fills the entire display with either on/off.
    \param on Sets on or off.
    */
    void Fill(bool on) override { driver_.Fill(on); }

    /**
    Sets the pixel at the specified coordinate to be on/off.
    \param x   x Coordinate
    \param y   y coordinate
    \param on  on or off
    */
    void DrawPixel(uint_fast8_t x, uint_fast8_t y, bool on) override
    {
        driver_.DrawPixel(x, y, on);
    }

    /** 
    Writes the current display buffer to the OLED device using SPI or I2C depending on 
    how the object was initialized.
    */
    void Update() override { driver_.Update(); }

  private:
    DisplayDriver driver_;

    void Reset() { driver_.Reset(); };
    void SendCommand(uint8_t cmd) { driver_.SendCommand(cmd); };
    void SendData(uint8_t* buff, size_t size) { driver_.SendData(buff, size); };
};

} // namespace daisy

#endif
