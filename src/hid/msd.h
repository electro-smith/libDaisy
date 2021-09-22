#ifndef DSY_MSD
#define DSY_MSD

#include <cstdint>

namespace daisy
{

/** Status of USB Host application
 * 
 */
typedef enum {
  APPLICATION_IDLE = 0,
  APPLICATION_START,
  APPLICATION_READY,
  APPLICATION_DISCONNECT
}ApplicationTypeDef;

/** 
   @author Gabriel Ball
   @date September 16, 2021

   @brief Presents a USB Mass Storage Device host interface
*/
class MSDHandle
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
     *  \param seed Pointer to initialized seed hardware class
     */
    Result Init();

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

    MSDHandle() : pimpl_(nullptr) {}
    MSDHandle(const MSDHandle& other) = default;
    MSDHandle& operator=(const MSDHandle& other) = default;

    class Impl; /**< & */

  private:

    Impl* pimpl_;

};

} // namespace daisy

#endif // DSY_MSD