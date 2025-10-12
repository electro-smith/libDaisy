/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

//--------------------------------------------------------------------+
// Board Specific Configuration
//--------------------------------------------------------------------+

// RHPort number used for device can be defined by board.mk, default to port 0
#define BOARD_TUD_FS_RHPORT 0
#define BOARD_TUD_HS_RHPORT 1

#ifndef BOARD_TUD_RHPORT
#define BOARD_TUD_RHPORT BOARD_TUD_FS_RHPORT
#endif

// RHPort max operational speed can defined by board.mk
#ifndef BOARD_TUD_MAX_SPEED
#define BOARD_TUD_MAX_SPEED OPT_MODE_DEFAULT_SPEED
#endif

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

// defined by board.mk
#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU OPT_MCU_STM32H7
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS OPT_OS_NONE
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG 0
#endif

// Enable Device stack
#define CFG_TUD_ENABLED 1

// Default is max speed that hardware controller could support with on-chip PHY
#define CFG_TUD_MAX_SPEED BOARD_TUD_MAX_SPEED

/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
// #define CFG_TUSB_MEM_SECTION __attribute__((section(".sram1_bss")))
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))
#endif

    //--------------------------------------------------------------------
    // DEVICE CONFIGURATION
    //--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE 64
#endif

//------------- CLASS -------------//
#define CFG_TUD_CDC 1
#define CFG_TUD_MSC 1
#define CFG_TUD_HID 0
#define CFG_TUD_MIDI 1
#define CFG_TUD_VENDOR 0

// CDC FIFO size of TX and RX
#define CFG_TUD_CDC_RX_BUFSIZE 64
#define CFG_TUD_CDC_TX_BUFSIZE 64

// CDC Endpoint transfer buffer size, more is faster
#define CFG_TUD_CDC_EP_BUFSIZE 64

// Number of virtual MIDI cables IN to the host
#define CFG_TUD_MIDI_NUMCABLES_IN 5
// Number of virtual MIDI cables OUT from the host
#define CFG_TUD_MIDI_NUMCABLES_OUT 1

// Support MIDI port string labels after the serial number string
// Set this to the first available string descriptor number or
// 0 if you do not wish to label the MIDI jacks with strings
#define CFG_TUD_MIDI_FIRST_PORT_STRIDX 5

// MIDI FIFO size of TX and RX
#define CFG_TUD_MIDI_RX_BUFSIZE 64
#define CFG_TUD_MIDI_TX_BUFSIZE 64

// MSC buffer size should be a multiple of (up to) 512 bytes for optimal performance
// Larger buffers = fewer transactions = better performance
#define CFG_TUD_MSC_EP_BUFSIZE 4096

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
