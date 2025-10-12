/*
 * USB MSC SD Card Interface Header
 * 
 * This file provides the interface for the USB Mass Storage Class
 * implementation that exposes the SD card as a USB drive.
 * 
 * The implementation is in usb_msc_sd.c and provides all required
 * TinyUSB MSC callbacks.
 */

#ifndef __USB_MSC_SD_H
#define __USB_MSC_SD_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

    // This file doesn't expose any public API
    // All functionality is provided through TinyUSB MSC callbacks
    // which are called automatically when the device is in MSC mode.

    // The implementation requires:
    // - CFG_TUD_MSC = 1 in tusb_config.h
    // - A valid SD card inserted and initialized
    // - The hsd1 handle to be available (standard libDaisy setup)

    // Note: When the device is enumerated in MSC mode, the application
    // should NOT access the SD card directly to avoid conflicts.

#ifdef __cplusplus
}
#endif

#endif /* __USB_MSC_SD_H */
