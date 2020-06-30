/*
TODO: 
- Implement configuration (currently all settings are fixed).
*/
#pragma once
#ifndef DSY_SDMMC_H
#define DSY_SDMMC_H /**< macro */

#include <stdint.h>

#define DSY_SD_OK 0    /**< OK return */
#define DSY_SD_ERROR 1 /**< ERROR return */


namespace daisy
{
/** @addtogroup other
    @{
    */

/** Operating Mode. Currently only FatFS is supported. */
enum SdmmcMode
{
    SDMMC_MODE_FATFS, /**< & */
};

/** Sets whether 4-bit mode or 1-bit mode is used for the SDMMC */
enum SdmmcBitWidth
{
    SDMMC_BITS_1, /**< & */
    SDMMC_BITS_4, /**< & */
};
/** 
    Sets the desired clock speed of the SD card bus. \n 
    Initialization is always done at or below 400kHz, and then the user speed is set.
*/
enum SdmmcSpeed
{
    SDMMC_SPEED_400KHZ, /**< & */
    SDMMC_SPEED_12MHZ,  /**< & */
};

/** Structure for setting the options above. Used to intiailize SdmmcHandler */
struct SdmmcHandlerInit
{
    SdmmcBitWidth bitdepth; /**< & */
    SdmmcSpeed    speed;    /**< & */
};

/** Configuration for interfacing with SD cards.
    Currently only supports operation using FatFS filesystem
*/
class SdmmcHandler
{
  public:
    SdmmcHandler() {}
    ~SdmmcHandler() {}

    /** Initializes the SD Card Interface For now all settings are fixed (See todo at top of section) */
    void Init();

  private:
};
/** @} */
} // namespace daisy

#endif
