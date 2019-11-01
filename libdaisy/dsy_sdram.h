// SDRAM for 32MB AS4C16M16SA (and 64MB equivalent).
// Thanks to whoever this awesome person is:
// http://main.lv/writeup/stm32f4_sdram_configuration.md
// The Init function is basically a copy and paste.
// He has references to timing, etc.
// for now we're configuring the RAM to run at 108MHz

// To use these the .sdram_data/_bss sections must be
//    configured correctly in the LINKER SCRIPT.
//    using BSS is advised for most things, since the DATA section must also fit in flash in order to be initialized.

/*
 * Edit the Linker Script to have the following lines in the following sections:

MEMORY
{
	SDRAM (RWX) : ORIGIN = 0xC0000000, LENGTH = 64M
}

SECTIONS
{
	.sdram_data :
	{
		. = ALIGN(4);
		_ssdram_data = .;

		PROVIDE(__sdram_data_start = _ssdram_data);
		*(.sdram_data)
		*(.sdram_data*)
		. = ALIGN(4);
		_esdram_data = .;

		PROVIDE(__sdram_data_end = _esdram_data);
	} > SDRAM AT >FLASH
	_sisdram_data = LOADADDR(.sdram_data);

	.sdram_bss (NOLOAD):
	{
		. = ALIGN(4);
		__ssdram_bss = .;
		PROVIDE(__sdram_bss_start = _ssdram_bss);
		*(.sdram_bss)
		*(.sdram_bss*)
		. = ALIGN(4);
		_esdram_bss = .;

		PROVIDE(__sdram_bss_end = _esdram_bss);
	} > SDRAM
}

* */

#ifndef RAM_AS4C16M16SA_H
#define RAM_AS4C16M16SA_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

//E.g. int SDRAM_DATA g_Initialized = 1;
#define DSY_SDRAM_DATA __attribute__((section(".sdram_data")))

//E.g. int SDRAM_BSS g_Uninitialized;
#define DSY_SDRAM_BSS __attribute__((section(".sdram_bss")))

enum
{
	DSY_SDRAM_OK,
	DSY_SDRAM_ERR,
};

// Determines whether chip is initialized, and activated.
typedef enum
{
	DSY_SDRAM_STATE_ENABLE,
	DSY_SDRAM_STATE_DISABLE,
	DSY_SDRAM_STATE_LAST,
} dsy_sdram_state;

// This is only the pins that can change on a board-to-board basis.
// Pins that have functions that cannot be moved to another pin will
// be hardcoded into the driver.
//
// - SDNWE is the only pin that i've seen move,
//		though I feel like there are probably a few more.
typedef enum
{
	DSY_SDRAM_PIN_SDNWE,
	DSY_SDRAM_PIN_LAST,
} dsy_sdram_pin;


typedef struct
{
	dsy_sdram_state state;
	dsy_gpio_pin	pin_config[DSY_SDRAM_PIN_LAST];
} dsy_sdram_handle_t;

//#include "main.h"
uint8_t dsy_sdram_init(dsy_sdram_handle_t *dsy_hsdram);
#ifdef __cplusplus
}
#endif

#endif
