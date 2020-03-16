#pragma once
#ifndef DSY_OLED_DISPLAY_H
#define DSY_OLED_DISPLAY_H
#include <stdlib.h>
#include <stdint.h>
#include "util_oled_fonts.h"

// Credit to akiskon/stm32-ssd1306 on github for a great starting point.
// For now Im hard coding everything, and then I'll move configurations out.

#ifndef SSD1309_HEIGHT
#define SSD1309_HEIGHT 64
#endif

// SSD1309 width in pixels
#ifndef SSD1309_WIDTH
#define SSD1309_WIDTH 128
#endif

namespace daisy
{
class OledDisplay
{
  public:
    OledDisplay() {}
    ~OledDisplay() {}
    void Init();
    void Fill(bool on);
    void DrawPixel(uint8_t x, uint8_t y, bool on);
    char WriteChar(char ch, FontDef font, bool on);
    char WriteString(char* str, FontDef font, bool on);
    void SetCursor(uint8_t x, uint8_t y);
    void Update();

  private:
    void Reset();
    void SendCommand(uint8_t byte);
    void SendData(uint8_t *buff, size_t size);
};
} // namespace daisy

#endif
