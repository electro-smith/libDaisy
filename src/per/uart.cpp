#include <stm32h7xx_hal.h>
#include "per/uart.h"
#include "util/ringbuffer.h"

using namespace daisy;

// Uncomment to use a second FIFO that is copied during the UART callback
// This will help with issues where data is overwritten while its being processed
// Cost:
// * 264 bytes (sizeof(UartRingBuffer)), to D1 RAM
// * 160 bytes on FLASH
// * Time to copy DMA FIFO to queue FIFO
#define UART_RX_DOUBLE_BUFFER 1

#define UART_RX_BUFF_SIZE 256

typedef RingBuffer<uint8_t, UART_RX_BUFF_SIZE> UartRingBuffer;
static UartRingBuffer DMA_BUFFER_MEM_SECTION   uart_dma_fifo;

static void Error_Handler()
{
    asm("bkpt 255");
}

// Uses HAL so these things have to be local to this file only
struct uart_handle
{
    UART_HandleTypeDef huart1;
    DMA_HandleTypeDef  hdma_usart1_rx;
    bool               receiving;
    size_t             rx_size, rx_last_pos;
    UartRingBuffer*    dma_fifo_rx;
    bool               rx_active, tx_active;
#ifdef UART_RX_DOUBLE_BUFFER
    UartRingBuffer queue_rx;
#endif
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
    uhandle.dma_fifo_rx = &uart_dma_fifo;
    uhandle.dma_fifo_rx->Init();
    uhandle.rx_size = UART_RX_BUFF_SIZE;
    // Buffer that gets copied
    uhandle.rx_active = false;
    uhandle.tx_active = false;
#ifdef UART_RX_DOUBLE_BUFFER
    uhandle.queue_rx.Init();
#endif
}

int UartHandler::PollReceive(uint8_t* buff, size_t size, uint32_t timeout)
{
    return HAL_UART_Receive(&uhandle.huart1, (uint8_t*)buff, size, timeout);
}

int UartHandler::StartRx()
{
    int status = 0;
    // Now start Rx
    status = HAL_UART_Receive_DMA(
        &uhandle.huart1,
        (uint8_t*)uhandle.dma_fifo_rx->GetMutableBuffer(),
        uhandle.rx_size);
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
#ifdef UART_RX_DOUBLE_BUFFER
    uhandle.queue_rx.Flush();
#else
    uhandle.dma_fifo_rx->Flush();
#endif
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
#ifdef UART_RX_DOUBLE_BUFFER
    return uhandle.queue_rx.Read();
#else
    return uhandle.dma_fifo_rx->Read();
#endif
}

size_t UartHandler::Readable()
{
#ifdef UART_RX_DOUBLE_BUFFER
    return uhandle.queue_rx.readable();
#else
    return uhandle.dma_fifo_rx->readable();
#endif
}

// Callbacks
static void UARTRxComplete(void)
{
    size_t len, cur_pos;
    //get current write pointer
    cur_pos = (uhandle.rx_size
               - ((DMA_Stream_TypeDef*)uhandle.huart1.hdmarx->Instance)->NDTR)
              & (uhandle.rx_size - 1);
    //calculate how far the DMA write pointer has moved
    len = (cur_pos - uhandle.rx_last_pos + uhandle.rx_size) % uhandle.rx_size;
    //check message size
    if(len <= uhandle.rx_size)
    {
        uhandle.dma_fifo_rx->Advance(len);
        uhandle.rx_last_pos = cur_pos;
#ifdef UART_RX_DOUBLE_BUFFER
        // Copy to queue fifo we don't want to use primary fifo to avoid
        // changes to the buffer while its being processed
        uint8_t processbuf[256];
        uhandle.dma_fifo_rx->ImmediateRead(processbuf, len);
        uhandle.queue_rx.Overwrite(processbuf, len);
#endif
    }
    else
    {
        while(1)
            ; //implement message to large exception
    }
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    if(huart->Instance == USART1)
    {
        if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
        {
            UARTRxComplete();
        }
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
        __HAL_UART_ENABLE_IT(&uhandle.huart1, UART_IT_IDLE);
        // Disable HalfTransfer Interrupt
        ((DMA_Stream_TypeDef*)uhandle.hdma_usart1_rx.Instance)->CR
            &= ~(DMA_SxCR_HTIE);

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
    void USART1_IRQHandler()
    {
        HAL_UART_IRQHandler(&uhandle.huart1);
        //        if(__HAL_UART_GET_FLAG(&uhandle.huart1, UART_FLAG_IDLE))
        //        {
        if((uhandle.huart1.Instance->ISR & UART_FLAG_IDLE) == UART_FLAG_IDLE)
        {
            HAL_UART_RxCpltCallback(&uhandle.huart1);
            //__HAL_UART_CLEAR_IDLEFLAG(&uhandle.huart1);
            uhandle.huart1.Instance->ICR = UART_FLAG_IDLE;
        }
    }

    void DMA1_Stream5_IRQHandler()
    {
        HAL_DMA_IRQHandler(&uhandle.hdma_usart1_rx);
    }
}
