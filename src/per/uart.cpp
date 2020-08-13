#include <stm32h7xx_hal.h>
#include "per/uart.h"
#include "util/ringbuffer.h"

using namespace daisy;

static void Error_Handler()
{
    asm("bkpt 255");
}

uint8_t DMA_BUFFER_MEM_SECTION uart_dma_buffer_rx[32];

// Uses HAL so these things have to be local to this file only
struct uart_handle
{
    UART_HandleTypeDef      huart1;
    DMA_HandleTypeDef       hdma_usart1_rx;
    uint8_t*                dma_buffer_rx;
    bool                    receiving;
    size_t                  rx_size;
    RingBuffer<uint8_t, 64> queue_rx;
    bool                    rx_active, tx_active;
};
static uart_handle uhandle;

void UartHandler::Init()
{
    uhandle.huart1.Instance                    = USART1;
    uhandle.huart1.Init.BaudRate               = 31250;
    uhandle.huart1.Init.WordLength             = UART_WORDLENGTH_8B;
    uhandle.huart1.Init.StopBits               = UART_STOPBITS_1;
    uhandle.huart1.Init.Parity                 = UART_PARITY_NONE;
    uhandle.huart1.Init.Mode                   = UART_MODE_TX_RX;
    uhandle.huart1.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    uhandle.huart1.Init.OverSampling           = UART_OVERSAMPLING_16;
    uhandle.huart1.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    uhandle.huart1.Init.ClockPrescaler         = UART_PRESCALER_DIV1;
    uhandle.huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if(HAL_UART_Init(&uhandle.huart1) != HAL_OK)
    {
        Error_Handler();
    }
    if(HAL_UARTEx_SetTxFifoThreshold(&uhandle.huart1, UART_TXFIFO_THRESHOLD_1_8)
       != HAL_OK)
    {
        Error_Handler();
    }
    if(HAL_UARTEx_SetRxFifoThreshold(&uhandle.huart1, UART_RXFIFO_THRESHOLD_1_8)
       != HAL_OK)
    {
        Error_Handler();
    }
    if(HAL_UARTEx_DisableFifoMode(&uhandle.huart1) != HAL_OK)
    {
        Error_Handler();
    }
    // Internal bits
    uhandle.dma_buffer_rx = uart_dma_buffer_rx;
    uhandle.queue_rx.Init();
    uhandle.rx_active = false;
    uhandle.tx_active = false;
}

int UartHandler::PollReceive(uint8_t* buff, size_t size, uint32_t timeout)
{
    return HAL_UART_Receive(&uhandle.huart1, (uint8_t*)buff, size, timeout);
}

int UartHandler::StartRx(size_t size)
{
    int status = 0;
    // Now start Rx
    uhandle.rx_size = size <= 32 ? size : 32;
    status          = HAL_UART_Receive_DMA(
        &uhandle.huart1, (uint8_t*)uhandle.dma_buffer_rx, size);
    if(status == 0)
        uhandle.rx_active = true;
    return status;
}

bool UartHandler::RxActive()
{
    return uhandle.rx_active;
}

int UartHandler::FlushRx()
{
    int status = 0;
    uhandle.queue_rx.Flush();
    return status;
}

int UartHandler::PollTx(uint8_t* buff, size_t size)

{
    return HAL_UART_Transmit(&uhandle.huart1, (uint8_t*)buff, size, 10);
}

int UartHandler::CheckError()
{
    return HAL_UART_GetError(&uhandle.huart1);
}

uint8_t UartHandler::PopRx()
{
    return uhandle.queue_rx.Read();
}

size_t UartHandler::Readable()
{
    return uhandle.queue_rx.readable();
}

// Callbacks

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    for(size_t i = 0; i < uhandle.rx_size; i++)
    {
        // TODO:
        // Add handling for non-writable, overflow conditions, etc.
        if(uhandle.queue_rx.writable())
            uhandle.queue_rx.Write(uhandle.dma_buffer_rx[i]);
        else
            uhandle.queue_rx.Overwrite(uhandle.dma_buffer_rx[i]);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart)
{
    switch(huart->ErrorCode)
    {
        case HAL_UART_ERROR_NONE: break;
        case HAL_UART_ERROR_PE: break;  // Parity Error
        case HAL_UART_ERROR_NE: break;  // Noise Error
        case HAL_UART_ERROR_FE: break;  // Frame Error
        case HAL_UART_ERROR_ORE: break; // Overrun Error
        case HAL_UART_ERROR_DMA: break; // DMA Transfer Erro
        default: break;
    }
    // Mark rx as deactivated
    uhandle.rx_active = false;
}
void HAL_UART_AbortCpltCallback(UART_HandleTypeDef* huart)
{
    //    asm("bkpt 255");
}
void HAL_UART_AbortTransmitCpltCallback(UART_HandleTypeDef* huart)
{
    //    asm("bkpt 255");
}
void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef* huart)
{
    //    asm("bkpt 255");
}

// Unimplemented HAL Callbacks
//void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart);
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
//void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart);


// HAL Interface functions
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
        GPIO_InitStruct.Pin       = GPIO_PIN_7 | GPIO_PIN_6;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* USART1 DMA Init */
        /* USART1_RX Init */
        uhandle.hdma_usart1_rx.Instance                 = DMA1_Stream5;
        uhandle.hdma_usart1_rx.Init.Request             = DMA_REQUEST_USART1_RX;
        uhandle.hdma_usart1_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        uhandle.hdma_usart1_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
        uhandle.hdma_usart1_rx.Init.MemInc              = DMA_MINC_ENABLE;
        uhandle.hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        uhandle.hdma_usart1_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        uhandle.hdma_usart1_rx.Init.Mode                = DMA_CIRCULAR;
        uhandle.hdma_usart1_rx.Init.Priority            = DMA_PRIORITY_LOW;
        uhandle.hdma_usart1_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
        if(HAL_DMA_Init(&uhandle.hdma_usart1_rx) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(uartHandle, hdmarx, uhandle.hdma_usart1_rx);

        /* USART1 interrupt Init */
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
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
        HAL_NVIC_DisableIRQ(USART1_IRQn);
        /* USER CODE BEGIN USART1_MspDeInit 1 */

        /* USER CODE END USART1_MspDeInit 1 */
    }
}

// HAL Interrupts.
extern "C"
{
    void USART1_IRQHandler() { HAL_UART_IRQHandler(&uhandle.huart1); }

    void DMA1_Stream5_IRQHandler()
    {
        HAL_DMA_IRQHandler(&uhandle.hdma_usart1_rx);
    }
}
