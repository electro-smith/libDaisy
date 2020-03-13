#pragma once
#ifndef DSY_BSP_SD_DISKIO_H
#define DSY_BSP_SD_DISKIO_H
#include <stdint.h>
#include "util_hal_map.h"

// SHIT MOVE THIS ALL TO ANOTHER 'BSP' FILE...
// Static Functions internal for diskIO
#define SD_CardInfo HAL_SD_CardInfoTypeDef
#define MSD_OK ((uint8_t)0x00)
#define MSD_ERROR ((uint8_t)0x01)
#define MSD_ERROR_SD_NOT_PRESENT ((uint8_t)0x02)
#define SD_TRANSFER_OK ((uint8_t)0x00)
#define SD_TRANSFER_BUSY ((uint8_t)0x01)
#define SD_PRESENT ((uint8_t)0x01)
#define SD_NOT_PRESENT ((uint8_t)0x00)
#define SD_DATATIMEOUT ((uint32_t)100000000)
#define SD_DetectIRQHandler() HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8) // I don't think this is right...

uint8_t BSP_SD_Init(void);
uint8_t BSP_SD_ITConfig(void);
uint8_t BSP_SD_ReadBlocks(uint32_t *pData,
                          uint32_t  ReadAddr,
                          uint32_t  NumOfBlocks,
                          uint32_t  Timeout);
uint8_t BSP_SD_WriteBlocks(uint32_t *pData,
                           uint32_t  WriteAddr,
                           uint32_t  NumOfBlocks,
                           uint32_t  Timeout);
uint8_t
        BSP_SD_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks);
uint8_t BSP_SD_WriteBlocks_DMA(uint32_t *pData,
                               uint32_t  WriteAddr,
                               uint32_t  NumOfBlocks);
uint8_t BSP_SD_Erase(uint32_t StartAddr, uint32_t EndAddr);
uint8_t BSP_SD_GetCardState(void);
void    BSP_SD_GetCardInfo(SD_CardInfo *CardInfo);
uint8_t BSP_SD_IsDetected(void);

// These functions can be modified in case the current settings (e.g. DMA stream)
//   need to be changed for specific application needs 
void BSP_SD_AbortCallback(void);
void BSP_SD_WriteCpltCallback(void);
void BSP_SD_ReadCpltCallback(void);

#endif

