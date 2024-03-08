#ifndef USB_HOST_H
#define USB_HOST_H

#include <cstdint>
#include "usbh_def.h"

namespace daisy
{
/** Status of USB Host application
 * 
 */
typedef enum
{
    APPLICATION_IDLE = 0,
    APPLICATION_START,
    APPLICATION_READY,
    APPLICATION_DISCONNECT
} ApplicationTypeDef;

/** 
   @author Gabriel Ball
   @date September 16, 2021

   @brief Presents a USB Mass Storage Device host interface
*/
class USBHostHandle
{
  public:
    /** @brief return codes from the USB Processing 
     *  can be used to check the state of USB while running
     *  outside of what may be happening with the limited user callbacks.
     * 
     *  At this time, these correlate directly to the ST Middleware
     *  USBH_StatusTypeDef codes
     */
    enum class Result
    {
        OK,
        BUSY,
        FAIL,
        NOT_SUPPORTED,
        UNRECOVERED_ERROR,
        ERROR_SPEED_UNKNOWN
    };

    /** @brief User defineable callback for USB Connection */
    typedef void (*ConnectCallback)(void* data);

    /** @brief User defineable callback for USB Disconnection */
    typedef void (*DisconnectCallback)(void* data);

    /** @brief User defineable callback upon completion of class initialization 
     *  For example, when a USB drive is connected and the usb device class
     *  initialization has finished, this callback will fire.
     * 
     *  @param userdata a pointer to some arbitrary data for use by the user.
     *   this is supplied in the Config struct. Can be used to avoid globals.
     * 
     *  @todo At some point this may be replaced for individual callbacks
     *   for each supported USB Host class.
     */
    typedef void (*ClassActiveCallback)(void* userdata);

    /** @brief User defineable callback for USB Unrecoverable Error 
     *  @todo add some sort of feedback about the type of error, etc.
     *   if possible
    */
    typedef void (*ErrorCallback)(void* data);

    /** @brief Configuration structure for interfacing with USB host Driver */
    struct Config
    {
        Config()
        : connect_callback(nullptr),
          disconnect_callback(nullptr),
          class_active_callback(nullptr),
          error_callback(nullptr),
          userdata(nullptr)
        {
        }
        ConnectCallback     connect_callback;
        DisconnectCallback  disconnect_callback;
        ClassActiveCallback class_active_callback;
        ErrorCallback       error_callback;
        void*               userdata;
    };

    /**
     * Register a USB class
     */
    Result RegisterClass(USBH_ClassTypeDef* pClass);

    /** Initializes the USB drivers and starts timeout.
     * 
     *  \param config Configuration struct for initialization
     */
    Result Init(USBHostHandle::Config& config);

    /** Deinitializes USB host-related peripherals
     * 
     */
    Result Deinit();

    /**
     * Returns true if the specified class is active
     */
    bool IsActiveClass(USBH_ClassTypeDef* usbClass);

    /** Manages usb host functionality
     * 
     */
    Result Process();

    /** Forces USB host to re-enumerate device */
    Result ReEnumerate();

    /** Returns true if a Mass Storage Device is connected
     *  and ready for communicaton
     * 
     */
    bool GetReady();

    /** Run after the first `Process` call to detect if
     *  a device is present
     * 
     */
    bool GetPresent();

    /** Returns name of the connected devices if there is one
     */
    const char* GetProductName();

    USBHostHandle() : pimpl_(nullptr) {}
    USBHostHandle(const USBHostHandle& other) = default;
    USBHostHandle& operator=(const USBHostHandle& other) = default;

    class Impl; /**< & */

  private:
    Impl* pimpl_;
};

} // namespace daisy

#endif // DSY_MSD
