#ifndef __fatfs_H
#define __fatfs_H /**< & */

#include "ff.h"
#include "ff_gen_drv.h"
#include "util/sd_diskio.h"
#include "util/usbh_diskio.h"

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
         *  TODO: FIX THE OR'ING Because that actually can't
         *   be done with the enum class on it's own..
         */
        enum class Media : uint8_t
        {
            SD   = 0x01,
            USBH = 0x02,
            // QSPI = 0x04,
        };
                          
        uint8_t media;
    };

    FatFSInterface() {}

    /** Link the desired hardware specified via Config::Media */
    Result Init(const Config& cfg)
    {
        Result ret = Result::ERR_NO_MEDIA_SELECTED;
        cfg_       = cfg;
        if(cfg_.media & static_cast<uint8_t>(Config::Media::SD))
            ret = FATFS_LinkDriver(&SD_Driver, path_[0]) == FR_OK
                      ? Result::OK
                      : Result::ERR_TOO_MANY_VOLUMES;
        if(cfg_.media & static_cast<uint8_t>(Config::Media::USBH))
            ret = FATFS_LinkDriver(&USBH_Driver, path_[1]) == FR_OK
                      ? Result::OK
                      : Result::ERR_TOO_MANY_VOLUMES;
        if(ret == Result::OK)
            initialized_ = true;
        return ret;
    }

    /** Unlinks FatFS from the configured media */
    Result DeInit()
    {
        Result ret = Result::ERR_NO_MEDIA_SELECTED;
        if(cfg_.media & static_cast<uint8_t>(Config::Media::SD))
            ret = FATFS_UnLinkDriver(path_[0]) == FR_OK
                      ? Result::OK
                      : Result::ERR_TOO_MANY_VOLUMES;
        if(cfg_.media & static_cast<uint8_t>(Config::Media::USBH))
            ret = FATFS_UnLinkDriver(path_[1]) == FR_OK
                      ? Result::OK
                      : Result::ERR_TOO_MANY_VOLUMES;
        if(ret == Result::OK)
            initialized_ = false;
        return ret;
    }

    bool Initialized() const { return initialized_; }

    /** Return the current configuration */
    const Config& GetConfig() const { return cfg_; }

    /** Return a mutable reference to the Configuration */
    Config& GetMutableConfig() { return cfg_; }

    /** Returns the path to an SD Card volume to use with f_mount */
    const char* GetSDPath() const { return path_[0]; }

    /** Returns the path to a USB Device volume to use with f_mount */
    const char* GetUSBPath() const { return path_[1]; }

  private:
    char   path_[2][4];
    Config cfg_;
    bool   initialized_;
};

} // namespace daisy

#endif
