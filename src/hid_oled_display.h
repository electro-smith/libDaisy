
#pragma once
#ifndef DSY_OLED_DISPLAY_H
#define DSY_OLED_DISPLAY_H
#include <stdlib.h>
#include <stdint.h>
#include "util_oled_fonts.h"
#include "daisy_core.h"


#ifndef SSD1309_HEIGHT
#define SSD1309_HEIGHT 64
#endif

/** SSD1309 width in pixels
*/
#ifndef SSD1309_WIDTH
#define SSD1309_WIDTH 128
#endif

namespace daisy
{

/** Human Interface Driver for using an OLED Display (SSD1309)
    For all `bool on` arguments: true is on, false is off.
    Credit to Aleksander Alekseev (github.com/afiskon/stm32-ssd1306) on github for a great starting point.
    adapted for SSD1309 and H7 by shensley, 2020
*/

class OledDisplay
{
  public:
    /** GPIO Pins that need to be used independent of peripheral used.
     */
    enum Pins
    {
      DATA_COMMAND, /** Data command pi. */
      RESET,        /** Reset pin */
      NUM_PINS,     /* Num pins */
    };
    OledDisplay() {}
    ~OledDisplay() {}
    /** TODO:
     */
    
    /** - add I2C Support.
	- add configuration for specific spi/i2c peripherals (currently only uses SPI1, w/ hardware controlled chip select.
	- re-add support for SSD1306 displays
    */
    
    /** Init
	Takes an argument for the pin cfg
	\pin_cfg should be a pointer to an array of OledDisplay::NUM_PINS dsy_gpio_pins
    */
    void Init(dsy_gpio_pin* pin_cfg);

    /** Fill
	Fills the entire display with either on/off.
	\param on Sets on or off.
    */
    void Fill(bool on);

    /** DrawPixel
	Sets the pixel at the specified coordinate to be on/off.
	\param x   x Coordinate
	\param y   y coordinate
	\param on  on or off
    */
    void DrawPixel(uint8_t x, uint8_t y, bool on);

    /** WriteChar
	Writes the character with the specific FontDef
	to the display buffer at the current Cursor position.
	\param char character to be written
	\param font font to be written in
	\bool on    on or off
    */
    char WriteChar(char ch, FontDef font, bool on);

    /** WriteString
	Similar to WriteChar, except it will handle an entire String.
	Wrapping does not happen automatically, so the width
	of the string must be kept within the dimensions of the screen.
	\param str string to be written
	\param font font to use
	\param on  on or off
    */
    char WriteString(char* str, FontDef font, bool on);

    /** SetCursor
	Moves the 'Cursor' position used for WriteChar, and WriteStr to the specified coordinate.
	\param x x pos
	\param y y pos
    */
    void SetCursor(uint8_t x, uint8_t y);

    /** Update
	Writes the current display buffer to the OLED device using SPI or I2C depending on 
	how the object was initialized.
    */
    void Update();

  private:
    void Reset();
    void SendCommand(uint8_t byte);
    void SendData(uint8_t* buff, size_t size);
};
} // namespace daisy

#endif
