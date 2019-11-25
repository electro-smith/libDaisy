#include "dsy_dma.h"

#ifdef __cplusplus
extern "C"
{
#endif 

void dsy_dma_init(void) 
{
  // DMA controller clock enable 
  __HAL_RCC_DMA1_CLK_ENABLE();

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
  // DMA1_Stream3_IRQn interrupt configuration
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

}

#ifdef __cplusplus
}
#endif
