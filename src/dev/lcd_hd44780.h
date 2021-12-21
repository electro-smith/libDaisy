#pragma once
#ifndef DSY_LCD_HD44780_H
#define DSY_LCD_HD44780_H

#include "daisy_core.h"
#include "per/gpio.h"


namespace daisy
{
/**
   @brief Device Driver for 16x2 LCD panel. \n 
   HD44780 with 4 data lines. \n
   Example product: https://www.adafruit.com/product/181
   @author StaffanMelin
   @date March 2021
   @ingroup device
*/
class LcdHD44780
{
  public:
    LcdHD44780() {}
    ~LcdHD44780() {}

    struct Config
    {
        bool         cursor_on;
        bool         cursor_blink;
        dsy_gpio_pin rs, en, d4, d5, d6, d7;
    };

    /** 
    Initializes the LCD.
     * \param config is a struct that sets cursor on/off, cursor blink on/off and the dsy_gpio_pin's that connects to the LCD.
     */
    void Init(const Config &config);

    /** 
    Prints a string on the LCD.
     * \param string is a C-formatted string to print.
     */
    void Print(const char *string);

    /** 
    Prints an integer value on the LCD.
     * \param number is an integer to print.
     */
    void PrintInt(int number);

    /** 
    Moves the cursor of the LCD (the place to print the next value).
     * \param row is the row number (0 or 1).
     * \param col is the column number (0 to 15).
     */
    void SetCursor(uint8_t row, uint8_t col);

    /** 
    Clears the contents of the LCD.
     */
    void Clear();

  private:
    bool     cursor_on;
    bool     cursor_blink;
    dsy_gpio lcd_pin_rs;
    dsy_gpio lcd_pin_en;
    dsy_gpio lcd_data_pin[4]; // D4-D7

    void WriteData(uint8_t);
    void WriteCommand(uint8_t);
    void Write(uint8_t, uint8_t);
};

} // namespace daisy

#endif
