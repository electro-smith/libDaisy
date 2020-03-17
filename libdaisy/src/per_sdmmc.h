#pragma once
#ifndef DSY_SDMMC_H
#define DSY_SDMMC_H

#include <stdint.h>

#define DSY_SD_OK 0
#define DSY_SD_ERROR 1

namespace daisy
{
enum SdmmcMode
{
    SDMMC_MODE_FATFS,
};
enum SdmmcBitWidth
{
    SDMMC_BITS_1,
    SDMMC_BITS_4,
};
enum SdmmcSpeed
{
    SDMMC_SPEED_400KHZ,
    SDMMC_SPEED_12MHZ,
};
struct SdmmcHandlerInit
{
    SdmmcBitWidth bitdepth;
    SdmmcSpeed    speed;
};
class SdmmcHandler
{
  public:
    SdmmcHandler() {}
    ~SdmmcHandler() {}

    void Init();

  private:
};
} // namespace daisy

#endif
