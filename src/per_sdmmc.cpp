#include "per_sdmmc.h"
#include "util_hal_map.h"
//#include "util_bsp_sd_diskio.h"
// Actual FatFS
//#include "ff.h"
//#include "ff_gen_drv.h"

#include "fatfs.h"


using namespace daisy;

SD_HandleTypeDef hsd1;
// Fat stuff
//uint8_t retSD;
//char    SDPath[4];
//FATFS   SDFatFS;
//FIL     SDFile;
//// Driver Link
// TODO: Probably separate this into drivers for
// SD diskIO and FATFS instead of lumping it all together.
// Especially since FATFS can be set up on the SDRAM, QSPI,
//  USB, and SD Card.

void SdmmcHandler::Init()
{
    hsd1.Instance                 = SDMMC1;
    hsd1.Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
    hsd1.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd1.Init.BusWide             = SDMMC_BUS_WIDE_4B;
    hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    //  hsd1.Init.ClockDiv = 168; // 476.2kHz works
    hsd1.Init.ClockDiv = 6; // 12MHz works
                            // FatFs stuff.
    //  dsy_fatfs_init();
}


// HAL MSP Functions

void HAL_SD_MspInit(SD_HandleTypeDef* sdHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(sdHandle->Instance == SDMMC1)
    {
        /* USER CODE BEGIN SDMMC1_MspInit 0 */

        /* USER CODE END SDMMC1_MspInit 0 */
        /* SDMMC1 clock enable */
        __HAL_RCC_SDMMC1_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**SDMMC1 GPIO Configuration    
    PC12     ------> SDMMC1_CK
    PC11     ------> SDMMC1_D3
    PC10     ------> SDMMC1_D2
    PD2     ------> SDMMC1_CMD
    PC9     ------> SDMMC1_D1
    PC8     ------> SDMMC1_D0 
    */
        GPIO_InitStruct.Pin
            = GPIO_PIN_12 | GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_8;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin       = GPIO_PIN_2;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* SDMMC1 interrupt Init */
        HAL_NVIC_SetPriority(SDMMC1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
        /* USER CODE BEGIN SDMMC1_MspInit 1 */

        /* USER CODE END SDMMC1_MspInit 1 */
    }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef* sdHandle)
{
    if(sdHandle->Instance == SDMMC1)
    {
        /* USER CODE BEGIN SDMMC1_MspDeInit 0 */

        /* USER CODE END SDMMC1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_SDMMC1_CLK_DISABLE();

        /**SDMMC1 GPIO Configuration    
    PC12     ------> SDMMC1_CK
    PC11     ------> SDMMC1_D3
    PC10     ------> SDMMC1_D2
    PD2     ------> SDMMC1_CMD
    PC9     ------> SDMMC1_D1
    PC8     ------> SDMMC1_D0 
    */
        HAL_GPIO_DeInit(GPIOC,
                        GPIO_PIN_12 | GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_9
                            | GPIO_PIN_8);

        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

        /* SDMMC1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(SDMMC1_IRQn);
        /* USER CODE BEGIN SDMMC1_MspDeInit 1 */

        /* USER CODE END SDMMC1_MspDeInit 1 */
    }
}
