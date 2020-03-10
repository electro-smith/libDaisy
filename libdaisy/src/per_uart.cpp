#include <stm32h7xx_hal.h>
#include "per_uart.h"

// TODO:
// - Add flexible parameters (for now just hard-coded for MIDI...)

using namespace daisy;

struct uart_handle
{
    UART_HandleTypeDef huart1;
    DMA_HandleTypeDef  hdma_usart1_rx;
};

static uart_handle h_uart;

void UartHandler::Init()
{
    h_uart.huart1.Instance      = USART1;
    h_uart.huart1.Init.BaudRate = 31250;
        h_uart.huart1.Init.WordLength
            = UART_WORDLENGTH_8B; // 1 Start, 8 data, 1 stop
    h_uart.huart1.Init.StopBits = UART_STOPBITS_1;
    h_uart.huart1.Init.Parity   = UART_PARITY_NONE;
    h_uart.huart1.Init.Mode     = UART_MODE_TX_RX;
    h_uart.huart1.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    h_uart.huart1.Init.OverSampling           = UART_OVERSAMPLING_16;
    h_uart.huart1.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    h_uart.huart1.Init.ClockPrescaler         = UART_PRESCALER_DIV16;
    h_uart.huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if(HAL_UART_Init(&h_uart.huart1) != HAL_OK)
    {
        //        Error_Handler();
    }
    if(HAL_UARTEx_SetTxFifoThreshold(&h_uart.huart1, UART_TXFIFO_THRESHOLD_1_8)
       != HAL_OK)
    {
        //        Error_Handler();
    }
    if(HAL_UARTEx_SetRxFifoThreshold(&h_uart.huart1, UART_RXFIFO_THRESHOLD_1_8)
       != HAL_OK)
    {
        //        Error_Handler();
    }
    if(HAL_UARTEx_DisableFifoMode(&h_uart.huart1) != HAL_OK)
    {
        //        Error_Handler();
    }

    //HAL_UART_Init(&h_uart.huart1);
}

int UartHandler::PollReceive(uint8_t* buff, size_t size)
{
    return HAL_UART_Receive(&h_uart.huart1, (uint8_t*)buff, size, 1);
}


// Msp Functions for HAL
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(uartHandle->Instance == USART1)
    {
        /* USER CODE BEGIN USART1_MspInit 0 */

        /* USER CODE END USART1_MspInit 0 */
        /* USART1 clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**USART1 GPIO Configuration    
    PB7     ------> USART1_RX
    PB6     ------> USART1_TX 
    */
//        GPIO_InitStruct.Pin       = GPIO_PIN_6;
//        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pin       = GPIO_PIN_7 | GPIO_PIN_6;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//        GPIO_InitStruct.Pin       = GPIO_PIN_7;
//        GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
//        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* USART1 DMA Init */
        /* USART1_RX Init */
        h_uart.hdma_usart1_rx.Instance                 = DMA1_Stream4;
        h_uart.hdma_usart1_rx.Init.Request             = DMA_REQUEST_USART1_RX;
        h_uart.hdma_usart1_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        h_uart.hdma_usart1_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
        h_uart.hdma_usart1_rx.Init.MemInc              = DMA_MINC_ENABLE;
        h_uart.hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        h_uart.hdma_usart1_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        h_uart.hdma_usart1_rx.Init.Mode                = DMA_NORMAL;
        h_uart.hdma_usart1_rx.Init.Priority            = DMA_PRIORITY_LOW;
        h_uart.hdma_usart1_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
        if(HAL_DMA_Init(&h_uart.hdma_usart1_rx) != HAL_OK)
        {
            //            Error_Handler();
        }

        __HAL_LINKDMA(uartHandle, hdmarx, h_uart.hdma_usart1_rx);

        /* USER CODE BEGIN USART1_MspInit 1 */

        /* USER CODE END USART1_MspInit 1 */
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
    if(uartHandle->Instance == USART1)
    {
        /* USER CODE BEGIN USART1_MspDeInit 0 */

        /* USER CODE END USART1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();

        /**USART1 GPIO Configuration    
    PB7     ------> USART1_RX
    PB6     ------> USART1_TX 
    */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7 | GPIO_PIN_6);

        /* USART1 DMA DeInit */
        HAL_DMA_DeInit(uartHandle->hdmarx);
        /* USER CODE BEGIN USART1_MspDeInit 1 */

        /* USER CODE END USART1_MspDeInit 1 */
    }
}
