/*
 * USB Descriptors Header
 *
 * Provides interface for configuring USB device modes before initialization
 */

#ifndef __USB_DESCRIPTORS_H
#define __USB_DESCRIPTORS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/**
 * USB Mode constants
 */
#define USB_MODE_NORMAL 0      // CDC + MIDI (music production)
#define USB_MODE_PROGRAMMING 1 // CDC + MSC (firmware updates with logging)

    /**
 * Set the USB mode before USB initialization
 *
 * This function MUST be called before initializing the USB peripheral
 * or any subsystem that initializes USB (e.g., Logger with USB CDC).
 *
 * CRITICAL: Call this BEFORE:
 *   - tusb_init()
 *   - hw.InitUsb()
 *   - Logger::StartLog() (if using USB CDC)
 *   - Any other USB initialization
 *
 * @param mode USB_MODE_NORMAL (0) for CDC+MIDI, USB_MODE_PROGRAMMING (1) for CDC+MSC
 *
 * @note Changing the mode after USB has been initialized will have no effect
 *       until the device is reset and re-enumerated.
 *
 * Example usage:
 * @code
 *   // 1. Initialize hardware (GPIO, etc.)
 *   hw.Init();
 *
 *   // 2. Check programming mode switch
 *   if (is_programming_mode_enabled()) {
 *       usb_set_mode(USB_MODE_PROGRAMMING);
 *   } else {
 *       usb_set_mode(USB_MODE_NORMAL);
 *   }
 *
 *   // 3. NOW safe to initialize USB-dependent systems
 *   START_LOG();  // Logger may init USB CDC
 *   hw.InitUsb(); // Main USB init
 * @endcode
 */
    void usb_set_mode(uint32_t mode);

#ifdef __cplusplus
}
#endif

#endif /* __USB_DESCRIPTORS_H */
