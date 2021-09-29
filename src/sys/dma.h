/** @addtogroup system
    @{
*/

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

    /** Deinitializes the DMA (specifically for the modules used within the library) */
    void dsy_dma_deinit(void);

    /** DMA transfers require the buffers to be excluded from the cache because the DMA
     *  reads / writes directly to the SRAM whereas the processor itself accesses the cache.
     *  Otherwise the DMA will access whatever is in the SRAM at the time which may not be 
     *  in sync with the data in the cache - resulting in wrong data transmitted / received. 
     *  You can place buffers in the D2 memory domain, in a section that has the cache disabled
     *  like this:
     *      uint8_t DMA_BUFFER_MEM_SECTION my_buffer[100];
     *  If this is not possible for some reason, call this function to clear the cache (write 
     *  cache contents to SRAM if required) before starting to transmit data via the DMA.
     */
    void dsy_dma_clear_cache_for_buffer(uint8_t* buffer, size_t size);

    /** DMA transfers require the buffers to be excluded from the cache because the DMA
     *  reads / writes directly to the SRAM whereas the processor itself accesses the cache.
     *  Otherwise the DMA will access whatever is in the SRAM at the time which may not be 
     *  in sync with the data in the cache - resulting in wrong data transmitted / received. 
     *  You can place buffers in the D2 memory domain, in a section that has the cache disabled
     *  like this:
     *      uint8_t DMA_BUFFER_MEM_SECTION my_buffer[100];
     *  If this is not possible for some reason, call this function to invalidate the cache (read 
     *  SRAM contents to cache if required) after reading data from peripherals via the DMA.
     */
    void dsy_dma_invalidate_cache_for_buffer(uint8_t* buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
