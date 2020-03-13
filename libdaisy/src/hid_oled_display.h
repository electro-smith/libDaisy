#pragma once
#ifndef DSY_OLED_DISPLAY_H
#define DSY_OLED_DISPLAY_H

// Credit to akiskon/stm32-ssd1306 on github for a great starting point.
// For now Im hard coding everything, and then I'll move configurations out.
#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT 64
#endif

// SSD1306 width in pixels
#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH 128
#endif

namespace daisy
{
class OledDisplay
{
  public:
    OledDisplay() {}
    ~OledDisplay() {}
    void Init();

    void Reset();
    void Fill(bool on);
    void Update();

  private:
};
} // namespace daisy

#endif
