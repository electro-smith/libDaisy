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

    // Callback function types
    typedef void (*usb_msc_callback_t)(void);

    /**
     * @brief Set callback to be executed when USB MSC device is unmounted (ejected)
     *
     * This callback is triggered when the host safely ejects the USB drive
     * (e.g., "Eject" on macOS, "Safely Remove Hardware" on Windows).
     * It's called from the MSC Start/Stop Unit SCSI command handler when
     * load_eject=1 and start=0.
     *
     * @param callback Function to call when unmounted, or NULL to unregister
     */
    void usb_msc_set_unmount_callback(usb_msc_callback_t callback);

    /**
     * @brief Set callback to be executed when USB device is disconnected
     *
     * This callback is triggered when the USB cable is physically disconnected
     * or the device is suspended/disconnected by the host.
     * It's called from the TinyUSB suspend callback (tud_suspend_cb).
     * Note: This may also be called on USB suspend, not just physical disconnect.
     *
     * @param callback Function to call when disconnected, or NULL to unregister
     */
    void usb_msc_set_disconnect_callback(usb_msc_callback_t callback);

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
