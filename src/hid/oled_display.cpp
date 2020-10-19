#include "hid/oled_display.h"
#include "daisy_core.h"
#include "per/spi.h"
#include "per/gpio.h"
#include "sys/system.h"


// Set up for now with:
// SCL -> PG11 (SPI1_SCK)
// SDA -> PB4 (SPI1_MOSI)
// RES -> PB8 (I2C1_SDA)
// DC -> PB9 (I2C1_SCL)

// Commands
//
// For now only the single-byte commands are notated.
// Some others are here with info on what the other bytes are.
// A few of the more complicated or mask-based commands have
// not been populated yet.
//
// Fundamental Commands (SSD1309 datasheet page 27)
//
// Requires 8-bit data for contrast 1-256
#define SSD1309_CMD_SET_CONTRAST (0x81)
#define SSD1309_CMD_ENTIRE_DISPLAY_ON (0xa4)
#define SSD1309_CMD_SET_NORMAL_DISPLAY (0xa6)
#define SSD1309_CMD_SET_INVERSE_DISPLAY (0xa7)
#define SSD1309_CMD_SET_DISPLAY_OFF (0xae)
#define SSD1309_CMD_SET_DISPLAY_ON (0xaf)
#define SSD1309_CMD_SET_NOP (0xe3)

// TODO: Add notation for using this.
#define SSD1309_CMD_SET_COMMAND_LOCK (0xfd)

// Scrolling Commands (added as needed, some may still be missing (SSD1309 datasheett page 29)

// Addressing Setting Commands (added as needed, some may still be missing (SSD1309 datasheett page 29)

// takes a byte after from lsb to msb {horiontal, vertical, page, invalid}
#define SSD1309_CMD_SET_MEMORY_ADDRESSING_MODE (0x20)

// Hardware Configuration
#define SSD1309_CMD_SET_COM_OUTPUT_SCAN_DIR_NORMAL (0xc0)
#define SSD1309_CMD_SET_COM_OUTPUT_SCAN_DIR_REMAPPED (0xc8)
#define SSD1309_CMD_SET_SEGMENT_REMAP_NORMAL (0xa0)
#define SSD1309_CMD_SET_SEGMENT_REMAP_MIRROR (0xa1)
// below commands require 1-64 as 0x00 to 0x3f byte following.
#define SSD1309_CMD_SET_MUX_RATIO (0xa8)
#define SSD1309_CMD_SET_DISPLAY_OFFSET (0xd3)

// See page 33 of datasheet for usage of below (resets to 0x12)
#define SSD1309_CMD_SET_COM_PIN_HW_CONFIG (0xda)

// Timing and Driving Scheme Setting Commands

// The following requires a byte with the following data to follow:
// [7:4] Osc frequency && [3:0] Divide ratio as ([3:0] + 1)
#define SSD1309_CMD_SET_CLOCK_DIVIDE_RATIO (0xd5)

// The following requires a byte with the following data to follow:
// [7:4] Phase 2 period of up to 15 DCLK (resets to 0x02)
// [3:0] Phase 1 period of up to 15 DCLK (resets to 0x02)
#define SSD1309_CMD_SET_PRECHARGE_PERIOD (0xd9)

// The following nibble should be at [5:2]
// Hex codes for whole bytes:
// 0x00 - ~0.64 x Vcc
// 0x34 - ~0.78 x Vcc (default @ RESET)
// 0x3c - ~0.84 x Vcc
#define SSD1309_CMD_SET_VCOMH_DESEL_LVL (0xdb)

#ifndef deg2rad
#define deg2rad(deg) ((deg)*3.141592 / 180.0)
#endif


using namespace daisy;

static uint8_t SSD1309_Buffer[SSD1309_WIDTH * SSD1309_HEIGHT / 8];

typedef struct
{
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t  Inverted;
    uint8_t  Initialized;
} SSD1309_t;
static SpiHandle h_spi;
static dsy_gpio  pin_reset, pin_dc;
static SSD1309_t SSD1309;


void OledDisplay::Init(dsy_gpio_pin* pin_cfg)
{
    // Initialize both GPIO
    pin_dc.mode = DSY_GPIO_MODE_OUTPUT_PP;
    pin_dc.pin  = pin_cfg[OledDisplay::DATA_COMMAND];
    dsy_gpio_init(&pin_dc);
    pin_reset.mode = DSY_GPIO_MODE_OUTPUT_PP;
    pin_reset.pin  = pin_cfg[OledDisplay::RESET];
    dsy_gpio_init(&pin_reset);
    // Initialize SPI
    h_spi.Init();
    // Reset and Configure OLED.
    Reset();
    //SendCommand(0xAE); // display off
    SendCommand(SSD1309_CMD_SET_DISPLAY_OFF);
    // Set Memory Addressing Mode to Horizontal
    SendCommand(SSD1309_CMD_SET_MEMORY_ADDRESSING_MODE);
    SendCommand(0x00);
    // Set Page Start Address (Page 0) -- only valid in Page Addressing Mode.
    SendCommand(0xB0);
    // Set COM Output Scan Direction
    SendCommand(SSD1309_CMD_SET_COM_OUTPUT_SCAN_DIR_REMAPPED);
    // Set Low and then High Column Address nibbles
    SendCommand(0x00); // set low common address
    SendCommand(0x10); // set high column address
    // Set Display RAM dispaly start line register from 0-63
    // start line register resets to 0b000000 on RESET.
    SendCommand(0x40);
    //--set contrast control register - CHECK
    SendCommand(SSD1309_CMD_SET_CONTRAST);
    SendCommand(0xFF); // 255

    SendCommand(SSD1309_CMD_SET_SEGMENT_REMAP_MIRROR); // previously 'MIRROR'
    SendCommand(SSD1309_CMD_SET_NORMAL_DISPLAY);

    //Set Multiplex Ratio 1-64 (3f = 64)
    SendCommand(SSD1309_CMD_SET_MUX_RATIO);
    SendCommand(0x3F);

    SendCommand(SSD1309_CMD_ENTIRE_DISPLAY_ON);

    SendCommand(SSD1309_CMD_SET_DISPLAY_OFFSET); //-set display offset - CHECK
    SendCommand(0x00);                           //-not offset

    SendCommand(SSD1309_CMD_SET_CLOCK_DIVIDE_RATIO);
    SendCommand(0xF0); // (reset value is 0x70)

    SendCommand(SSD1309_CMD_SET_PRECHARGE_PERIOD);
    SendCommand(0x22); // defaults after reset

    SendCommand(
        SSD1309_CMD_SET_COM_PIN_HW_CONFIG); //--set com pins hardware configuration - CHECK
    SendCommand(0x12);

    SendCommand(SSD1309_CMD_SET_VCOMH_DESEL_LVL); //--set vcomh
    SendCommand(0x20);                            //0x20,0.77xVcc

    // Not sure if these two commands do anything on the SSD1309...
    //    SendCommand(0x8D); //--set DC-DC enable
    //    SendCommand(0x14); //

    SendCommand(SSD1309_CMD_SET_DISPLAY_ON); //--turn on SSD1309 panel
    Fill(false);
    Update();
    // Set default values for screen object
    SSD1309.CurrentX = 0;
    SSD1309.CurrentY = 0;

    SSD1309.Initialized = 1;
}
void OledDisplay::Reset()
{
    dsy_gpio_write(&pin_reset, 0);
    dsy_system_delay(10);
    dsy_gpio_write(&pin_reset, 1);
    dsy_system_delay(10);
}

void OledDisplay::Fill(bool on)
{
    for(size_t i = 0; i < sizeof(SSD1309_Buffer); i++)
    {
        SSD1309_Buffer[i] = on ? 0xff : 0x00;
    }
}

void OledDisplay::Update(void)
{
    uint8_t i;
    for(i = 0; i < 8; i++)
    {
        SendCommand(0xB0 + i);
        SendCommand(0x00);
        SendCommand(0x10);
        SendData(&SSD1309_Buffer[SSD1309_WIDTH * i], SSD1309_WIDTH);
    }
}

void OledDisplay::DrawPixel(uint_fast8_t x, uint_fast8_t y, bool on)
{
    if(x >= SSD1309_WIDTH || y >= SSD1309_HEIGHT)
        return;
    if(SSD1309.Inverted)
        on = !on;
    if(on)
        SSD1309_Buffer[x + (y / 8) * SSD1309_WIDTH] |= (1 << (y % 8));
    else
        SSD1309_Buffer[x + (y / 8) * SSD1309_WIDTH] &= ~(1 << (y % 8));
}

void OledDisplay::DrawLine(uint_fast8_t x1,
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

void OledDisplay::DrawRect(uint_fast8_t x1,
                           uint_fast8_t y1,
                           uint_fast8_t x2,
                           uint_fast8_t y2,
                           bool         on,
                           bool         fill)
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

void OledDisplay::DrawArc(uint_fast8_t x,
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

char OledDisplay::WriteChar(char ch, FontDef font, bool on)
{
    uint32_t i, b, j;

    // Check if character is valid
    if(ch < 32 || ch > 126)
        return 0;

    // Check remaining space on current line
    if(SSD1309_WIDTH < (SSD1309.CurrentX + font.FontWidth)
       || SSD1309_HEIGHT < (SSD1309.CurrentY + font.FontHeight))
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
                DrawPixel(SSD1309.CurrentX + j, (SSD1309.CurrentY + i), on);
            }
            else
            {
                DrawPixel(SSD1309.CurrentX + j, (SSD1309.CurrentY + i), !on);
            }
        }
    }

    // The current space is now taken
    SSD1309.CurrentX += font.FontWidth;

    // Return written char for validation
    return ch;
}
char OledDisplay::WriteString(const char* str, FontDef font, bool on)
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
void OledDisplay::SetCursor(uint8_t x, uint8_t y)
{
    SSD1309.CurrentX = x;
    SSD1309.CurrentY = y;
}

void OledDisplay::SendCommand(uint8_t byte)
{
    dsy_gpio_write(&pin_dc, 0);
    h_spi.BlockingTransmit(&byte, 1);
}

void OledDisplay::SendData(uint8_t* buff, size_t size)
{
    dsy_gpio_write(&pin_dc, 1);
    h_spi.BlockingTransmit(buff, size);
}
