#include "sys/fatfs.h"
#include "ff_gen_drv.h"
#include "util/sd_diskio.h"
#include "util/usbh_diskio.h"


using namespace daisy;

FatFSInterface::Result FatFSInterface::Init(const FatFSInterface::Config& cfg)
{
    Result ret = Result::ERR_NO_MEDIA_SELECTED;
    cfg_       = cfg;
    if(cfg_.media & Config::MEDIA_SD)
        ret = FATFS_LinkDriver(&SD_Driver, path_[0]) == FR_OK
                  ? Result::OK
                  : Result::ERR_TOO_MANY_VOLUMES;
    if(cfg_.media & Config::MEDIA_USB)
        ret = FATFS_LinkDriver(&USBH_Driver, path_[1]) == FR_OK
                  ? Result::OK
                  : Result::ERR_TOO_MANY_VOLUMES;
    if(ret == Result::OK)
        initialized_ = true;
    return ret;
}

FatFSInterface::Result FatFSInterface::Init(const uint8_t media)
{
    cfg_.media = media;
    return Init(cfg_);
}


FatFSInterface::Result FatFSInterface::DeInit()
{
    Result ret = Result::ERR_NO_MEDIA_SELECTED;
    if(cfg_.media & Config::MEDIA_SD)
        ret = FATFS_UnLinkDriver(path_[0]) == FR_OK
                  ? Result::OK
                  : Result::ERR_TOO_MANY_VOLUMES;
    if(cfg_.media & Config::MEDIA_USB)
        ret = FATFS_UnLinkDriver(path_[1]) == FR_OK
                  ? Result::OK
                  : Result::ERR_TOO_MANY_VOLUMES;
    if(ret == Result::OK)
        initialized_ = false;
    return ret;
}

extern "C"
{
    DWORD get_fattime(void) { return 0; }
}
