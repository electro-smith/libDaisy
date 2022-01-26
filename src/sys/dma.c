#include "stm32h7xx_hal.h"
#include "sys/dma.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void dsy_dma_init(void)
    {
        // DMA controller clock enable
        __HAL_RCC_DMA1_CLK_ENABLE();
        __HAL_RCC_DMA2_CLK_ENABLE();

        // DMA interrupt init
        // DMA1_Stream0_IRQn interrupt configuration
        HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
        // DMA1_Stream1_IRQn interrupt configuration
        HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
        // DMA1_Stream2_IRQn interrupt configuration
        HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
        // DMA1_Stream3_IRQn interrupt configuration
        HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
        // DMA1_Stream4_IRQn interrupt configuration
        HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
        // DMA1_Stream5_IRQn interrupt configuration
        HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
        // DMA1_Stream6_IRQn interrupt configuration for I2C
        HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
        // DMA2_Stream0_IRQn, interrupt configuration for DAC Ch1
        HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
        // DMA2_Stream1_IRQn, interrupt configuration for DAC Ch2
        HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

        // DMA2_Stream2_IRQn and DMA2_Stream3_IRQn interrupt configuration for SPI
        HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
        HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
    }

    void dsy_dma_deinit(void)
    {
        // DMA controller clock enable
        __HAL_RCC_DMA1_CLK_DISABLE();
        __HAL_RCC_DMA2_CLK_DISABLE();

        // DMA interrupt init
        // DMA1_Stream0_IRQn interrupt configuration
        HAL_NVIC_DisableIRQ(DMA1_Stream0_IRQn);
        // DMA1_Stream1_IRQn interrupt configuration
        HAL_NVIC_DisableIRQ(DMA1_Stream1_IRQn);
        // DMA1_Stream2_IRQn interrupt configuration
        HAL_NVIC_DisableIRQ(DMA1_Stream2_IRQn);
        // DMA1_Stream3_IRQn interrupt configuration
        HAL_NVIC_DisableIRQ(DMA1_Stream3_IRQn);
        // DMA1_Stream4_IRQn interrupt configuration
        HAL_NVIC_DisableIRQ(DMA1_Stream4_IRQn);
        // DMA1_Stream5_IRQn interrupt configuration
        HAL_NVIC_DisableIRQ(DMA1_Stream5_IRQn);
        // DMA1_Stream6_IRQn interrupt configuration for I2C
        HAL_NVIC_DisableIRQ(DMA1_Stream6_IRQn);
        // DMA2_Stream0_IRQn, interrupt configuration for DAC Ch1
        HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
        // DMA2_Stream1_IRQn, interrupt configuration for DAC Ch2
        HAL_NVIC_DisableIRQ(DMA2_Stream1_IRQn);

        // DMA2_Stream2_IRQn and DMA2_Stream3_IRQn interrupt configuration for SPI
        HAL_NVIC_DisableIRQ(DMA2_Stream2_IRQn);
        HAL_NVIC_DisableIRQ(DMA2_Stream3_IRQn);
    }

    void dsy_dma_clear_cache_for_buffer(uint8_t* buffer, size_t size)
    {
        // clear all cache lines (32bytes each) that span the memory section
        // of our transmit buffer. This makes sure that the SRAM contains the
        // most recent version of the buffer.
        SCB_CleanDCache_by_Addr(
            (uint32_t*)((uint32_t)(buffer) & ~(uint32_t)0x1F), size + 32);
    }

    void dsy_dma_invalidate_cache_for_buffer(uint8_t* buffer, size_t size)
    {
        // invalidate all cache lines (32bytes each) that span the memory section
        // of our transmit buffer. This makes sure that the cache contains the
        // most recent version of the buffer.
        SCB_InvalidateDCache_by_Addr(
            (uint32_t*)((uint32_t)(buffer) & ~(uint32_t)0x1F), size + 32);
    }

#ifdef __cplusplus
}
#endif
