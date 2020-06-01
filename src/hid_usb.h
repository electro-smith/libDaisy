/** Interface for initializing and using the USB Peripherals on the daisy
*/
//
/** **Author:** Stephen Hensley
*/
//
/** **Date Added:** December 2019
*/
//
#pragma once
#ifndef DSY_HID_USB_H
#define DSY_HID_USB_H
#include <stdint.h>
#include <stdlib.h>
namespace daisy
{
class UsbHandle
{
  public:
    /** Specified which of the two USB Peripherals to initialize.
*/
    //
    /** FS External D- pin is Pin 37 (GPIO31)
*/
    //
    /** FS External D+ pin is Pin 38 (GPIO32)
*/
    //
    enum UsbPeriph
    {
        FS_INTERNAL,
        FS_EXTERNAL,
        FS_BOTH,
    };

    /** Function called upon reception of a buffer
*/
    typedef void (*ReceiveCallback)(uint8_t* buff, uint32_t* len);

    UsbHandle() {}

    ~UsbHandle() {}

    /** Initializes the specified peripheral(s) as USB CDC Devices
*/
    void Init(UsbPeriph dev);

    /** Transmits a buffer of 'size' bytes from the on board USB FS port.
*/
    void TransmitInternal(uint8_t* buff, size_t size);
    /** Transmits a buffer of 'size' bytes from a USB port connected to the
external USB Pins of the daisy seed.
*/
    void TransmitExternal(uint8_t* buff, size_t size);

    /** sets the callback to be called upon reception of new data
*/
    void SetReceiveCallback(ReceiveCallback cb);

  private:
};

} // namespace daisy
#endif

/** - Add support for other USB classes (currently only CDC is supported)
- DMA setup
*/
