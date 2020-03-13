#include "hid_oled_display.h"
#include "daisy_core.h"
#include "per_spi.h"
#include "per_gpio.h"
#include "sys_system.h"


// Set up for now with:
// SCL -> PG11 (SPI1_SCK)
// SDA -> PB4 (SPI1_MOSI)
// RES -> PB8 (I2C1_SDA)
// DC -> PB9 (I2C1_SCL)

using namespace daisy;

static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

typedef struct
{
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t  Inverted;
    uint8_t  Initialized;
} SSD1306_t;
static SpiHandle    h_spi;
static dsy_gpio     pin_reset, pin_dc;
static dsy_gpio_pin cfg_reset = {DSY_GPIOB, 8};
static dsy_gpio_pin cfg_dc    = {DSY_GPIOB, 9};

static void SendCommand(uint8_t byte);
static void SendData(uint8_t* buff, size_t size);

static SSD1306_t SSD1306;


void OledDisplay::Init()
{
    // Initialize both GPIO
    pin_reset.mode = DSY_GPIO_MODE_OUTPUT_PP;
    pin_reset.pin  = cfg_reset;
    dsy_gpio_init(&pin_reset);
    pin_dc.mode    = DSY_GPIO_MODE_OUTPUT_PP;
    pin_dc.pin     = cfg_dc;
    dsy_gpio_init(&pin_dc);
    Reset();
    SendCommand(0xAE); // display off
    SendCommand(0x20); // Set Memory Addressing Mode
    SendCommand(
        0x00); // 00b Horizontal, 01b Vertical, 10b Page Address (RESET), 11b Invalid
    SendCommand(0xB0); // Set Page Start Address for Page Addressing Mode 0-7
    SendCommand(0xc8); // Set COM Output Scan Direction
    SendCommand(0x00); // set low common address
    SendCommand(0x10); // set high column address
    SendCommand(0x40); //--set start line address - CHECK

    SendCommand(0x81); //--set contrast control register - CHECK
    SendCommand(0xFF);

#ifdef SSD1306_MIRROR_HORIZ
    SendCommand(0xA0); // Mirror horizontally
#else
    SendCommand(0xA1); //--set segment re-map 0 to 127 - CHECK
#endif

#ifdef SSD1306_INVERSE_COLOR
    SendCommand(0xA7); //--set inverse color
#else
    SendCommand(0xA6); //--set normal color
#endif

    SendCommand(0xA8); //--set multiplex ratio(1 to 64) - CHECK
    SendCommand(0x3F); //

    SendCommand(
        0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content

    SendCommand(0xD3); //-set display offset - CHECK
    SendCommand(0x00); //-not offset

    SendCommand(
        0xD5); //--set display clock divide ratio/oscillator frequency
    SendCommand(0xF0); //--set divide ratio

    SendCommand(0xD9); //--set pre-charge period
    SendCommand(0x22); //

    SendCommand(0xDA); //--set com pins hardware configuration - CHECK
    SendCommand(0x12);

    SendCommand(0xDB); //--set vcomh
    SendCommand(0x20); //0x20,0.77xVcc

    SendCommand(0x8D); //--set DC-DC enable
    SendCommand(0x14); //
    SendCommand(0xAF); //--turn on SSD1306 panel
    Fill(false);
    Update();
    // Set default values for screen object
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;

    SSD1306.Initialized = 1;
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
    for(size_t i = 0; i < sizeof(SSD1306_Buffer); i++) {
        SSD1306_Buffer[i] = on ? 0xff : 0x00;
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
        SendData(&SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH);
    }
}
 
static void SendCommand(uint8_t byte)
{
    dsy_gpio_write(&pin_dc, 0);
    h_spi.BlockingTransmit(&byte, 1);
}
static void SendData(uint8_t* buff, size_t size)
{
    dsy_gpio_write(&pin_dc, 1);
    h_spi.BlockingTransmit(buff, size);
}
