#pragma once
#ifndef DSY_LCD_HD44780_H
#define DSY_LCD_HD44780_H

#include "daisy_core.h"
#include "per/gpio.h"


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

    void Init(const Config &);
    void Print(const char *);
    void PrintInt(int);
    void SetCursor(uint8_t, uint8_t);
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


#endif