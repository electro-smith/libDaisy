#pragma once
#ifndef DSY_HID_USB_H
#define DSY_HID_USB_H
#include <stdint.h>
#include <stdlib.h>

namespace daisy
/** @addtogroup human_interface
    @{
*/

{
/** 
    @brief Interface for initializing and using the USB Peripherals on the daisy
    @author Stephen Hensley
    @date December 2019
*/
class UsbHandle
{
  public:
    /** Return values for USBHandle Functions */
    enum class Result
    {
        OK,
        ERR,
    };

    /** Specified which of the two USB Peripherals to initialize. */
    enum UsbPeriph
    {
        FS_INTERNAL, /**< Internal pin */
        FS_EXTERNAL, /**< FS External D+ pin is Pin 38 (GPIO32). FS External D- pin is Pin 37 (GPIO31) */
        FS_BOTH,     /**< Both */
    };


    /** Function called upon reception of a buffer */
    typedef void (*ReceiveCallback)(uint8_t* buff, uint32_t* len);

    UsbHandle() {}

    ~UsbHandle() {}

    /** Initializes the specified peripheral(s) as USB CDC Devices
    \param dev Device to initialize
     */
    void Init(UsbPeriph dev);

    /** Deinitializes the specified peripheral(s)
    \param dev Device to deinitialize
     */
    void DeInit(UsbPeriph dev);

    /** Transmits a buffer of 'size' bytes from the on board USB FS port. 
    \param buff Buffer to transmit
    \param size Buffer size
     */
    Result TransmitInternal(uint8_t* buff, size_t size);
    /** Transmits a buffer of 'size' bytes from a USB port connected to the
    external USB Pins of the daisy seed.
    \param buff Buffer to transmit
    \param size Buffer size
    */
    Result TransmitExternal(uint8_t* buff, size_t size);

    /** sets the callback to be called upon reception of new data
    \param cb Function to serve as callback
    \param dev Device to set callback for
     */
    void SetReceiveCallback(ReceiveCallback cb, UsbPeriph dev);

  private:
};

} // namespace daisy
#endif

/* - Add support for other USB classes (currently only CDC is supported)
- DMA setup
*/
/** @} */
