#ifndef __fatfs_H
#define __fatfs_H /**< & */

#include "ff.h"

namespace daisy
{
/** @brief Daisy FatFS Driver Interface
 *  @details Specifies the desired media (SD Card, USB, etc.) to be mountable with FatFS
 *           within a given application. Once initialization is called, the standard
 *           FatFS API will be usable with the media mounted in the order 
 *           they are listed in the Media config struct. 
 *           For example, when only using an SD Card, it will mount at "0:/",
 *           when only using USB, it will mount at "0:/". However, when mounting 
 *           with SD card and USB, the SD card will mount at "0:/", and the USB 
 *           will mount at "1:/".
 *           The relevant hardware peripheral (SDMMC, or MSD) needs to be initialized
 *           separately by the application before using FatFS.
 *          
 *           This object has some memory limitations due to the media connected to it.
 *           The SDMMC1 peripheral can only communicate with the AXI SRAM, and the DTCMRAM 
 *           cannot communicate with the DMA. So the FatFSInterface object should always be
 *           located in the AXI SRAM. This is the default location for all data/bss memory
 *           using the standard build. However, applciations using the electrosmith bootloader
 *           will need special consideration when using this object AND an SD Card. 
 */
class FatFSInterface
{
  public:
    /** Return values specifying specific errors for linking Daisy to FatFS */
    enum Result
    {
        OK,
        ERR_TOO_MANY_VOLUMES,
        ERR_NO_MEDIA_SELECTED,
        ERR_GENERIC,
    };

    /** Config structure for configuring Daisy to FatFS */
    struct Config
    {
        /** Selected Media that will be linked to FatFS 
         *  These values can be OR'd together when multiple
         *  volumes are desired 
         *  i.e. config.media = Media::SD | Media::USBH;
         * 
         *  When mounting multiple volumes, ffconf.h must have 
         *  _VOLUMES set to an appropriate value.
         * 
         *  FatFS will register multiple volumes in the order of the enum,
         *   the first registered class will mount at "0:/",
         *   the second registered class will mount at "1:/", and so on
         */
        enum Media : uint8_t
        {
            MEDIA_SD  = 0x01,
            MEDIA_USB = 0x02,
            // MEDIA_QSPI = 0x04,
        };

        uint8_t media;
    };

    FatFSInterface() {}

    /** Link the desired hardware specified via Config::Media */
    Result Init(const Config& cfg);

    /** Link the desired hardware specified via Config::Media 
     * 
     *  Alternate, explicit initialization provided for simplified syntax.
     */
    Result Init(const uint8_t media);

    /** Unlinks FatFS from the configured media */
    Result DeInit();

    bool Initialized() const { return initialized_; }

    /** Return the current configuration */
    const Config& GetConfig() const { return cfg_; }

    /** Return a mutable reference to the Configuration */
    Config& GetMutableConfig() { return cfg_; }

    /** Returns the path to an SD Card volume to use with f_mount */
    const char* GetSDPath() const { return path_[0]; }

    /** Returns the path to a USB Device volume to use with f_mount */
    const char* GetUSBPath() const { return path_[1]; }

    /** Returns reference to filesystem object for the SD volume. */
    FATFS& GetSDFileSystem() { return fs_[0]; }

    /** Returns reference to filesystem object for the USB volume. */
    FATFS& GetUSBFileSystem() { return fs_[1]; }

  private:
    Config cfg_;
    FATFS  fs_[_VOLUMES];
    char   path_[_VOLUMES][4];
    bool   initialized_;
};

} // namespace daisy

/** Implementation of FatFS time method 
 *  @return 0
*/
extern "C" DWORD get_fattime(void);

#endif
