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

#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_midi_multi.h"

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))
#define USB_PID                                                      \
    (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) \
     | _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4))

#define USB_VID 0xCafe     // TODO: Replace with Nopia VID once we get one
#define USB_BCD 0x0200     // USB 2.0
#define USB_DEV_BCD 0x0100 // v1.0

// Configuration mode
// 0 : enumerated as CDC/MIDI (normal mode)
// 1 : enumerated as MSC (programming/firmware update mode)
// For now, this is controlled by NOPIA_PROGRAMMING_MODE macro
// TODO: Replace with actual button/GPIO check
#ifdef NOPIA_PROGRAMMING_MODE
static uint32_t usb_mode = 1;
#else
static uint32_t usb_mode = 0;
#endif

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+

// Device descriptor for mode 0 (CDC + MIDI)
tusb_desc_device_t const desc_device_0 = {
    .bLength         = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB          = USB_BCD,

    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass    = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor  = USB_VID,
    .idProduct = USB_PID,
    .bcdDevice = USB_DEV_BCD,

    .iManufacturer = 0x01,
    .iProduct      = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01,
};

// Device descriptor for mode 1 (MSC only)
tusb_desc_device_t const desc_device_1 = {
    .bLength         = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB          = USB_BCD,
    .bDeviceClass    = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,

    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor        = USB_VID,
    .idProduct       = USB_PID + 10, // Different PID for MSC mode
    .bcdDevice       = USB_DEV_BCD,

    .iManufacturer = 0x01,
    .iProduct      = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01,
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)(usb_mode ? &desc_device_1 : &desc_device_0);
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

// Mode 0: CDC + MIDI
enum
{
    ITF_0_NUM_CDC = 0,
    ITF_0_NUM_CDC_DATA,
    ITF_0_NUM_MIDI,
    ITF_0_NUM_MIDI_STREAMING,
    ITF_0_NUM_TOTAL
};

// Mode 1: MSC only
enum
{
    ITF_1_NUM_MSC = 0,
    ITF_1_NUM_TOTAL
};

#ifndef CFG_TUD_MIDI_NUMCABLES_IN
#define CFG_TUD_MIDI_NUMCABLES_IN 1
#endif

#ifndef CFG_TUD_MIDI_NUMCABLES_OUT
#define CFG_TUD_MIDI_NUMCABLES_OUT 1
#endif

#define CONFIG_0_TOTAL_LEN                                  \
    (TUD_CONFIG_DESC_LEN + (TUD_CDC_DESC_LEN * CFG_TUD_CDC) \
     + TUD_MIDI_MULTI_DESC_LEN(CFG_TUD_MIDI_NUMCABLES_IN,   \
                               CFG_TUD_MIDI_NUMCABLES_OUT))

#define CONFIG_1_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN)

// Endpoint numbers for mode 0 (CDC + MIDI)
#define EPNUM_0_CDC_NOTIF 0x81
#define EPNUM_0_CDC_OUT 0x02
#define EPNUM_0_CDC_IN 0x82

#define EPNUM_0_MIDI_OUT 0x03
#define EPNUM_0_MIDI_IN 0x83

// Endpoint numbers for mode 1 (MSC)
#define EPNUM_1_MSC_OUT 0x01
#define EPNUM_1_MSC_IN 0x81

// Configuration descriptor for mode 0 (CDC + MIDI)
uint8_t const desc_configuration_0[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_0_NUM_TOTAL, 0, CONFIG_0_TOTAL_LEN, 0x00, 100),

    // CDC: Interface number, string index, EP notification address and size, EP
    // data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_0_NUM_CDC,
                       4,
                       EPNUM_0_CDC_NOTIF,
                       8,
                       EPNUM_0_CDC_OUT,
                       EPNUM_0_CDC_IN,
                       64),

    // MIDI: Interface number, string index, EP Out & EP In address, EP size
    TUD_MIDI_MULTI_DESCRIPTOR(ITF_0_NUM_MIDI,
                              0,
                              EPNUM_0_MIDI_OUT,
                              EPNUM_0_MIDI_IN,
                              64,
                              CFG_TUD_MIDI_NUMCABLES_IN,
                              CFG_TUD_MIDI_NUMCABLES_OUT),
};

// Configuration descriptor for mode 1 (MSC)
uint8_t const desc_configuration_1[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_1_NUM_TOTAL, 0, CONFIG_1_TOTAL_LEN, 0x00, 100),

    // Interface number, string index, EP Out & EP In address, EP size
    TUD_MSC_DESCRIPTOR(ITF_1_NUM_MSC, 0, EPNUM_1_MSC_OUT, EPNUM_1_MSC_IN, 64),
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index; // for multiple configurations
    return usb_mode ? desc_configuration_1 : desc_configuration_0;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// String Descriptor Index
enum
{
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
};

// array of pointer to string descriptors
char const *string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    "Nopia & Friends",          // 1: Manufacturer
    "Nopia",                    // 2: Product
    NULL,                       // 3: Serials will use unique ID if possible
    "Nopia CDC",                // 4: CDC Interface
    "Nopia MIDI Out Keys",      // 5: MIDI Interface ...
    "Nopia MIDI Out Bass",
    "Nopia MIDI Out Arp",
    "Nopia MIDI Out Synth A",
    "Nopia MIDI Out Synth B",
    "Nopia MIDI In",
};

static uint16_t _desc_str[32 + 1];

// Invoked when received GET STRING DESCRIPTOR request
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;
    size_t chr_count;

    switch(index)
    {
        case STRID_LANGID:
            memcpy(&_desc_str[1], string_desc_arr[0], 2);
            chr_count = 1;
            break;

        case STRID_SERIAL:
            chr_count = board_usb_get_serial(_desc_str + 1, 32);
            break;

        default:
            // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
            // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

            if(!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
                return NULL;

            const char *str = string_desc_arr[index];

            // Cap at max char
            chr_count = strlen(str);
            size_t const max_count
                = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1;
            if(chr_count > max_count)
                chr_count = max_count;

            // Convert ASCII string into UTF-16
            for(size_t i = 0; i < chr_count; i++)
            {
                _desc_str[1 + i] = str[i];
            }
            break;
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

    return _desc_str;
}
