#ifndef __fatfs_H
#define __fatfs_H /**< & */

#ifdef __cplusplus
extern "C"
{
#endif

#include "ff.h"
#include "ff_gen_drv.h"
#include "util/sd_diskio.h"

    /** 
    @brief fatfs support.
    */


    extern uint8_t retSD;                /**< & */
    extern char    SDPath[4];            /**< & */
    extern FATFS   SDFatFS;              /**< & */
    extern FIL     SDFile;               /**< & */
    void           dsy_fatfs_init(void); /**< & */
    void           dsy_fatfs_deinit(void);

#ifdef __cplusplus
}
#endif
#endif
