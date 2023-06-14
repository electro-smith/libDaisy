/*
  
Project: LCD library for Electrosmith's Daisy Seed
Description: LCD class for a 16x2 LCD using the Hitachi HD44780 driver chip
Author: Staffan Melin, staffan.melin@oscillator.se
License: The MIT License (MIT)
Version: 202101
Project site: http://www.oscillator.se/opensource
Based on: HD44780-Stm32HAL by Olivier Van den Eede (https://github.com/4ilo/HD44780-Stm32HAL)

*/


#include <stdio.h>
#include <string.h>

#include "daisy_core.h"
#include "sys/system.h"

#include "lcd_hd44780.h"


#define CLEAR_DISPLAY 0x01

#define ENTRY_MODE_SET 0x04
#define OPT_S 0x01   // Shift entire display to right
#define OPT_INC 0x02 // Cursor increment

#define DISPLAY_ON_OFF_CONTROL 0x08
#define OPT_D 0x04 // Turn on display
#define OPT_C 0x02 // Turn on cursor
#define OPT_B 0x01 // Turn on cursor blink

#define FUNCTION_SET 0x20
#define OPT_DL 0x10 // Set interface data length
#define OPT_N 0x08  // Set number of display lines
#define OPT_F 0x04  // Set alternate font
#define SETCGRAM_ADDR 0x040
#define SET_DDRAM_ADDR 0x80 // Set DDRAM address

#define LCD_NIB 4
#define LCD_BYTE 8
#define LCD_DATA_REG 1
#define LCD_COMMAND_REG 0


namespace daisy
{
// Init LCD

void LcdHD44780::Init(const Config& config)
{
    // init pins
    lcd_pin_rs.Init(config.rs, GPIO::Mode::OUTPUT);
    lcd_pin_en.Init(config.en, GPIO::Mode::OUTPUT);

    lcd_data_pin[0].Init(config.d4, GPIO::Mode::OUTPUT);
    lcd_data_pin[1].Init(config.d5, GPIO::Mode::OUTPUT);
    lcd_data_pin[2].Init(config.d6, GPIO::Mode::OUTPUT);
    lcd_data_pin[3].Init(config.d7, GPIO::Mode::OUTPUT);

    cursor_on    = config.cursor_on;
    cursor_blink = config.cursor_blink;

    System::Delay(1);

    // init LCD

    WriteCommand(0x33);                 // function set
    WriteCommand(0x32);                 // function set
    WriteCommand(FUNCTION_SET | OPT_N); // 4-bit mode, 2 lines

    System::Delay(1);

    WriteCommand(CLEAR_DISPLAY);
    WriteCommand(
        DISPLAY_ON_OFF_CONTROL | OPT_D | (cursor_on ? OPT_C : 0)
        | (cursor_blink ? OPT_B : 0));      // LCD on, cursor + blink settings
    WriteCommand(ENTRY_MODE_SET | OPT_INC); // Increment cursor

    System::Delay(1);
}


// Print string on current pos

void LcdHD44780::Print(const char* string)
{
    for(uint8_t i = 0; i < strlen(string); i++)
    {
        WriteData(string[i]);
    }
}


// Print number

void LcdHD44780::PrintInt(int number)
{
    char buffer[11];
    sprintf(buffer, "%d", number);

    Print(buffer);
}


// Set cursor position

void LcdHD44780::SetCursor(uint8_t row, uint8_t col)
{
    WriteCommand(SET_DDRAM_ADDR + (row == 1 ? 0x40 : 0) + col);
}


// Clear screen

void LcdHD44780::Clear()
{
    WriteCommand(CLEAR_DISPLAY);
}


// Private methods


// Write byte to command register

void LcdHD44780::WriteCommand(uint8_t command)
{
    lcd_pin_rs.Write(LCD_COMMAND_REG);

    Write((command >> 4), LCD_NIB);
    Write(command & 0x0F, LCD_NIB);
}


// Write byte to data register

void LcdHD44780::WriteData(uint8_t data)
{
    lcd_pin_rs.Write(LCD_DATA_REG);

    Write(data >> 4, LCD_NIB);
    Write(data & 0x0F, LCD_NIB);
}


// Write data

void LcdHD44780::Write(uint8_t data, uint8_t len)
{
    for(uint8_t i = 0; i < len; i++)
    {
        lcd_data_pin[i].Write((data >> i) & 0x01);
    }

    // toggle

    lcd_pin_en.Write(1);
    System::Delay(1);
    lcd_pin_en.Write(0);
}

} // namespace daisy
