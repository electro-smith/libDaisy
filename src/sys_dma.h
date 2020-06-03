/** @file sys_dma.h */

/** Initializes the Direct Memory Access Peripheral used by
many internal elements of libdaisy. */
#ifndef __dma_H
#define __dma_H

#ifdef __cplusplus
extern "C"
{
#endif

    /** Initializes the DMA (specifically for the modules used within the library) */
    void dsy_dma_init(void);


#ifdef __cplusplus
}
#endif

#endif
