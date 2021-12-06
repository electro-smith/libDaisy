#ifndef DSY_MSD
#define DSY_MSD

#include <cstdint>

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
    enum Result
    {
        OK = 0,
        ERR
    };

    /** Configuration structure for interfacing with MSD Driver */
    struct Config
    {
    };

    /** Initializes the USB drivers and starts timeout.
     * 
     *  \param config Configuration struct for initialization
     */
    Result Init(Config config);

    /** Deinitializes MSD-related peripherals
     * 
     */
    Result Deinit();

    /** Manages usb host functionality
     * 
     */
    void Process();

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

    USBHostHandle() : pimpl_(nullptr) {}
    USBHostHandle(const USBHostHandle& other) = default;
    USBHostHandle& operator=(const USBHostHandle& other) = default;

    class Impl; /**< & */

  private:
    Impl* pimpl_;
};

} // namespace daisy

#endif // DSY_MSD