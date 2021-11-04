#include "per/sdmmc.h"
#include "util/hal_map.h"
//#include "fatfs.h"


using namespace daisy;

/** Local HAL handle */
SD_HandleTypeDef hsd1;

SdmmcHandler::Result SdmmcHandler::Init(const Config& cfg)
{
    hsd1.Instance            = SDMMC1;
    hsd1.Init.ClockEdge      = SDMMC_CLOCK_EDGE_RISING;
    hsd1.Init.ClockPowerSave = cfg.clock_powersave
                                   ? SDMMC_CLOCK_POWER_SAVE_ENABLE
                                   : SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd1.Init.BusWide
        = cfg.width == BusWidth::BITS_1 ? SDMMC_BUS_WIDE_1B : SDMMC_BUS_WIDE_4B;
    hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;

    switch(cfg.speed)
    {
        case Speed::SLOW: hsd1.Init.ClockDiv = 250; break;
        case Speed::MEDIUM_SLOW: hsd1.Init.ClockDiv = 8; break;
        case Speed::STANDARD: hsd1.Init.ClockDiv = 4; break;
        case Speed::FAST: hsd1.Init.ClockDiv = 2; break;
        case Speed::VERY_FAST: hsd1.Init.ClockDiv = 1; break;
    }
    return Result::OK;
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

        // Adjust the gpio drive strength based on the clock divider,
        // which is derived from the Speed config
        uint32_t gpioSpeed = GPIO_SPEED_FREQ_VERY_HIGH;
        switch(sdHandle->Init.ClockDiv)
        {
            case 250: gpioSpeed = GPIO_SPEED_FREQ_LOW; break;     // SLOW
            case 8: gpioSpeed = GPIO_SPEED_FREQ_MEDIUM; break;    // MEDIUM_SLOW
            case 4: gpioSpeed = GPIO_SPEED_FREQ_HIGH; break;      // STANDARD
            case 2: gpioSpeed = GPIO_SPEED_FREQ_VERY_HIGH; break; // FAST
            case 1: gpioSpeed = GPIO_SPEED_FREQ_VERY_HIGH; break; // VERY_FAST
        }

        GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_8;
        if(sdHandle->Init.BusWide == SDMMC_BUS_WIDE_4B)
            GPIO_InitStruct.Pin |= GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = gpioSpeed;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin       = GPIO_PIN_2;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = gpioSpeed;
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

extern "C"
{
    void SDMMC1_IRQHandler() { HAL_SD_IRQHandler(&hsd1); }
}
