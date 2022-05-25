/*
TODO: 
- Implement configuration (currently all settings are fixed).
*/
#pragma once
#ifndef DSY_SDMMC_H
#define DSY_SDMMC_H /**< macro */

#include <stdint.h>


namespace daisy
{
/** @ingroup peripheral
    @{
    */

/** Configuration for interfacing with SD cards.
 *  Currently only supports operation using FatFS filesystem
 * 
 *  Only SDMMC1 is supported at this time.
 *
 *  Pins are fixed to the following:
 *  PC12 - SDMMC1 CK
 *  PD2  - SDMMC1 CMD
 *  PC8  - SDMMC1 D0
 *  PC9  - SDMMC1 D1 (optional)
 *  PC10 - SDMMC1 D2 (optional)
 *  PC11 - SDMMC1 D3 (optional)
*/
class SdmmcHandler
{
  public:
    /** Return values for the SdmmcHandler class */
    enum class Result
    {
        OK,
        ERROR,
    };

    /** Sets whether 4-bit mode or 1-bit mode is used for the SDMMC */
    enum class BusWidth
    {
        BITS_1, /**< Only 1 bit of data per clock is transferred */
        BITS_4, /**< 4-bits of parallel data for each clock pulse */
    };

    /** 
    Sets the desired clock speed of the SD card bus. \n 
    Initialization is always done at or below 400kHz, and then the user speed is set.
    */
    enum class Speed
    {
        SLOW, /**< 400kHz, initialization  performed at this rate, before moving to user value */
        MEDIUM_SLOW, /**< 12.5MHz - half of standard rate */
        STANDARD,    /**< 25MHz - DS (Default Speed for SDMMC) */
        FAST,        /**< 50MHz - HS (High Speed signaling) */
        VERY_FAST, /**< 100MHz - SDR50 Overclocked rate for maximum transfer rates */
    };

    struct Config
    {
        Speed    speed;
        BusWidth width;
        bool
            clock_powersave; /**< When true, the clock will stop between transfers to save power. */

        /** Configures settings to their default settings */
        void Defaults()
        {
            speed           = Speed::FAST;
            width           = BusWidth::BITS_4;
            clock_powersave = false;
        }
    };

    SdmmcHandler() {}
    ~SdmmcHandler() {}

    /** Configures the SDMMC Peripheral with the user defined settings.
     * Initialization does not happen immediatly and will be called by the 
     * filesystem (i.e. FatFS).
     */
    Result Init(const Config& cfg);

  private:
};
/** @} */
} // namespace daisy

#endif
