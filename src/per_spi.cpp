#include "per_spi.h"
#include "util_hal_map.h"

// TODO
// - fix up rest of lib so that we can add a spi_handle map to the hal map
// - Add configuration for standard spi stuff.


using namespace daisy;

static SPI_HandleTypeDef hspi1;

static void Error_Handler()
{
    asm("bkpt 255");
}

void SpiHandle::Init()
{
    hspi1.Instance               = SPI1;
    hspi1.Init.Mode              = SPI_MODE_MASTER;
    hspi1.Init.Direction         = SPI_DIRECTION_2LINES_TXONLY;
    hspi1.Init.DataSize          = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity       = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase          = SPI_PHASE_1EDGE;
    hspi1.Init.NSS               = SPI_NSS_HARD_OUTPUT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi1.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode            = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial     = 0x0;
    hspi1.Init.NSSPMode          = SPI_NSS_PULSE_ENABLE;
    hspi1.Init.NSSPolarity       = SPI_NSS_POLARITY_LOW;
    hspi1.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_01DATA;
    hspi1.Init.TxCRCInitializationPattern
        = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi1.Init.RxCRCInitializationPattern
        = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi1.Init.MasterSSIdleness        = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi1.Init.MasterReceiverAutoSusp  = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi1.Init.MasterKeepIOState       = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    hspi1.Init.IOSwap                  = SPI_IO_SWAP_DISABLE;
    if(HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }
}

void SpiHandle::BlockingTransmit(uint8_t* buff, size_t size)
{
    HAL_SPI_Transmit(&hspi1, buff, size, 100);
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(spiHandle->Instance == SPI1)
    {
        /* USER CODE BEGIN SPI1_MspInit 0 */

        /* USER CODE END SPI1_MspInit 0 */
        /* SPI1 clock enable */
        __HAL_RCC_SPI1_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        /**SPI1 GPIO Configuration    
    PB5     ------> SPI1_MOSI
    PB4 (NJTRST)     ------> SPI1_MISO
    PG11     ------> SPI1_SCK
    PG10     ------> SPI1_NSS 
    */
        //        GPIO_InitStruct.Pin       = GPIO_PIN_5 | GPIO_PIN_4;
        switch(spiHandle->Init.Direction)
        {
            case SPI_DIRECTION_2LINES_TXONLY:
                GPIO_InitStruct.Pin = GPIO_PIN_5;
                break;
            case SPI_DIRECTION_2LINES_RXONLY:
                GPIO_InitStruct.Pin = GPIO_PIN_4;
                break;
            default: GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5; break;
        }
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // Sck and CS
        GPIO_InitStruct.Pin = GPIO_PIN_11;
        if(spiHandle->Init.NSS != SPI_NSS_SOFT)
        {
            GPIO_InitStruct.Pin |= GPIO_PIN_10;
        }
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

        /* USER CODE BEGIN SPI1_MspInit 1 */

        /* USER CODE END SPI1_MspInit 1 */
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{
    if(spiHandle->Instance == SPI1)
    {
        /* USER CODE BEGIN SPI1_MspDeInit 0 */

        /* USER CODE END SPI1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_SPI1_CLK_DISABLE();

        /**SPI1 GPIO Configuration    
    PB5     ------> SPI1_MOSI
    PB4 (NJTRST)     ------> SPI1_MISO
    PG11     ------> SPI1_SCK
    PG10     ------> SPI1_NSS 
    */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5 | GPIO_PIN_4);

        HAL_GPIO_DeInit(GPIOG, GPIO_PIN_11 | GPIO_PIN_10);

        /* USER CODE BEGIN SPI1_MspDeInit 1 */

        /* USER CODE END SPI1_MspDeInit 1 */
    }
}
