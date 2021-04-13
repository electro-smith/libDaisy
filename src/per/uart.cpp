#include <stm32h7xx_hal.h>
#include "per/uart.h"
#include "util/ringbuffer.h"
extern "C"
{
#include "util/hal_map.h"
}

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
	while(1){ }
}

class UartHandler::Impl
{
    public:
        UartHandler::Result Init(const UartHandler::Config& config);

        const UartHandler::Config& GetConfig() const { return config_; }
 
        int PollReceive(uint8_t *buff, size_t size, uint32_t timeout);

        UartHandler::Result StartRx();

        bool RxActive();

        UartHandler::Result FlushRx();

        UartHandler::Result PollTx(uint8_t *buff, size_t size);

        uint8_t PopRx();

        size_t Readable();

        int CheckError();

        UartHandler::Result InitPins();

        UartHandler::Result DeInitPins();

    GPIO_TypeDef* port;
    USART_TypeDef* periph;
    uint16_t tx, rx;

    UART_HandleTypeDef huart1;
    DMA_HandleTypeDef  hdma_usart1_rx;
    bool               receiving;
    size_t             rx_size, rx_last_pos;
    UartRingBuffer*    dma_fifo_rx;
    bool               rx_active, tx_active;
    #ifdef UART_RX_DOUBLE_BUFFER
        UartRingBuffer queue_rx;
    #endif

    UartHandler::Config config_;
};


// ================================================================
// Global references for the availabel UartHandler::Impl(s)
// ================================================================

static UartHandler::Impl uart_handles[9];

UartHandler::Impl* MapInstanceToHandle(USART_TypeDef* instance){
    constexpr USART_TypeDef* instances[9]
        = {USART1, USART2, USART3, UART4, UART5, USART6, UART7, UART8, LPUART1}; // map HAL instances
    for(int i = 0; i < 9; i++){
        if(instance == instances[i]){
            return &uart_handles[i];
        }
    }

    /* error */
    return NULL;
}

UartHandler::Result UartHandler::Impl::Init(const UartHandler::Config& config)
{
    config_ = config;

    const int uartIdx = int(config_.periph);
    if(uartIdx >= 9){ return Result::ERR; }
    constexpr USART_TypeDef* instances[9]
        = {USART1, USART2, USART3, UART4, UART5, USART6, UART7, UART8, LPUART1}; // map HAL instances
    periph = instances[uartIdx];

    const int parityIdx = int(config_.parity);
    if (parityIdx >= 3){ return Result::ERR; }
    constexpr uint32_t parity_[3] = {UART_PARITY_NONE, UART_PARITY_EVEN, UART_PARITY_ODD};

    const int stopbitsIdx = int(config_.stopbits);
    if (stopbitsIdx >= 4){ return Result::ERR; }
    constexpr uint32_t stop_bits_[4] = {UART_STOPBITS_0_5, UART_STOPBITS_1, UART_STOPBITS_1_5, UART_STOPBITS_2};

    const int modeIdx = int(config_.mode);
    if(modeIdx >= 3) { return Result::ERR;  }
    constexpr uint32_t mode_[3] = {UART_MODE_RX, UART_MODE_TX, UART_MODE_TX_RX};

    huart1.Instance                    = periph;
    huart1.Init.BaudRate               = config.baudrate;
    huart1.Init.WordLength             = UART_WORDLENGTH_8B;
    huart1.Init.StopBits               = stop_bits_[stopbitsIdx];
    huart1.Init.Parity                 = parity_[parityIdx];
    huart1.Init.Mode                   = mode_[modeIdx];
    huart1.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling           = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler         = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if(HAL_UART_Init(&huart1) != HAL_OK)
    {
        return Result::ERR;
    }
    if(HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8)
       != HAL_OK)
    {
        return Result::ERR;        
    }
    if(HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8)
       != HAL_OK)
    {
        return Result::ERR;        
    }
    if(HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
    {
        return Result::ERR;
    }

    // Internal bits
    dma_fifo_rx = &uart_dma_fifo;
    dma_fifo_rx->Init();
    rx_size = UART_RX_BUFF_SIZE;
    // Buffer that gets copied
    rx_active = false;
    tx_active = false;
    #ifdef UART_RX_DOUBLE_BUFFER
        queue_rx.Init();
    #endif

    return Result::OK;
}

int UartHandler::Impl::PollReceive(uint8_t* buff, size_t size, uint32_t timeout)
{
    return HAL_UART_Receive(&huart1, (uint8_t*)buff, size, timeout);
}

UartHandler::Result UartHandler::Impl::StartRx()
{
    int status = 0;
    // Now start Rx
    status = HAL_UART_Receive_DMA(
        &huart1,
        (uint8_t*)dma_fifo_rx->GetMutableBuffer(),
        rx_size);
    if(status == 0)
        rx_active = true;
    return rx_active ? Result::OK : Result::ERR;
}

bool UartHandler::Impl::RxActive()
{
    return rx_active;
}

//this originally had a useless status var hanging about
//I don't think we can actually error check this...
UartHandler::Result UartHandler::Impl::FlushRx()
{
    #ifdef UART_RX_DOUBLE_BUFFER
        queue_rx.Flush();
    #else
        dma_fifo_rx->Flush();
    #endif
    return Result::OK;
}

UartHandler::Result UartHandler::Impl::PollTx(uint8_t* buff, size_t size)
{
    HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1, (uint8_t*)buff, size, 10);
    return (status == HAL_OK ? Result::OK : Result::ERR);
}

int UartHandler::Impl::CheckError()
{
    return HAL_UART_GetError(&huart1);
}

uint8_t UartHandler::Impl::PopRx()
{
#ifdef UART_RX_DOUBLE_BUFFER
    return queue_rx.Read();
#else
    return dma_fifo_rx->Read();
#endif
}

size_t UartHandler::Impl::Readable()
{
#ifdef UART_RX_DOUBLE_BUFFER
    return queue_rx.readable();
#else
    return dma_fifo_rx->readable();
#endif
}

typedef struct{
    dsy_gpio_pin pin;
    uint8_t alt;
} pin_alt;

pin_alt pins_none = {{DSY_GPIOX, 0}, 255};

//valid pins per periph, and the alt they're on
pin_alt usart1_pins_tx[] = { {{DSY_GPIOB, 6}, GPIO_AF7_USART1}, {{DSY_GPIOB, 14}, GPIO_AF4_USART1}, pins_none };
pin_alt usart1_pins_rx[] = { {{DSY_GPIOB, 7}, GPIO_AF7_USART1}, {{DSY_GPIOB, 15}, GPIO_AF4_USART1}, pins_none };

pin_alt usart2_pins_tx[] = { {{DSY_GPIOA, 2}, GPIO_AF7_USART2}, pins_none, pins_none };
pin_alt usart2_pins_rx[] = { {{DSY_GPIOA, 3}, GPIO_AF7_USART2}, pins_none, pins_none };

pin_alt usart3_pins_tx[] = { {{DSY_GPIOC, 10}, GPIO_AF7_USART3}, pins_none, pins_none };
pin_alt usart3_pins_rx[] = { {{DSY_GPIOC, 11}, GPIO_AF7_USART3}, pins_none, pins_none };

pin_alt uart4_pins_tx[] = { {{DSY_GPIOB, 9}, GPIO_AF8_UART4}, {{DSY_GPIOC, 10}, GPIO_AF8_UART4}, pins_none };
pin_alt uart4_pins_rx[] = { {{DSY_GPIOB, 8}, GPIO_AF8_UART4}, {{DSY_GPIOC, 11}, GPIO_AF8_UART4}, pins_none };

pin_alt uart5_pins_tx[] = { {{DSY_GPIOC, 12}, GPIO_AF8_UART5 }, {{DSY_GPIOB, 6}, GPIO_AF14_UART5}, pins_none };
pin_alt uart5_pins_rx[] = { {{DSY_GPIOB, 12}, GPIO_AF14_UART5}, {{DSY_GPIOD, 2}, GPIO_AF8_UART5}, {{DSY_GPIOB, 5}, GPIO_AF14_UART5} };

pin_alt usart6_pins_tx[] = { pins_none, pins_none, pins_none };
pin_alt usart6_pins_rx[] = { {{DSY_GPIOG, 9}, GPIO_AF7_USART6}, pins_none, pins_none };

pin_alt uart7_pins_tx[] = { {{DSY_GPIOB, 4}, GPIO_AF11_UART7 }, pins_none, pins_none };
pin_alt uart7_pins_rx[] = { pins_none, pins_none, pins_none };

pin_alt uart8_pins_tx[] = { pins_none, pins_none, pins_none };
pin_alt uart8_pins_rx[] = { pins_none, pins_none, pins_none };

pin_alt lpuart1_pins_tx[] = { {{DSY_GPIOC, 6}, GPIO_AF8_LPUART}, pins_none, pins_none };
pin_alt lpuart1_pins_rx[] = { {{DSY_GPIOC, 7}, GPIO_AF8_LPUART}, pins_none, pins_none };

//an array to hold everything
pin_alt* pins_periphs[] = {
                            usart1_pins_tx, usart1_pins_rx, usart2_pins_tx, usart2_pins_rx, 
                            usart3_pins_tx, usart3_pins_rx,  uart4_pins_tx, uart4_pins_rx, 
                            uart5_pins_tx, uart5_pins_rx,  usart6_pins_tx, usart6_pins_rx, 
                            uart7_pins_tx, uart7_pins_rx,  uart8_pins_tx, uart8_pins_rx, 
                            lpuart1_pins_tx, lpuart1_pins_rx };

bool gpiopin_equal(dsy_gpio_pin a, dsy_gpio_pin b){
    return a.port == b.port && a.pin == b.pin;
}

UartHandler::Result checkPinMatch(GPIO_InitTypeDef* init, dsy_gpio_pin pin, int p_num){
    for(int i = 0; i < 3; i++){
        if(gpiopin_equal(pins_periphs[p_num][i].pin, pins_none.pin)) { 
            /* skip */
        }

        else if(gpiopin_equal(pins_periphs[p_num][i].pin, pin))
        {
            init->Alternate = pins_periphs[p_num][i].alt;
            return UartHandler::Result::OK;
        }
    }

    return UartHandler::Result::ERR;
}

UartHandler::Result UartHandler::Impl::InitPins()
{
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Mode  = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;    

    int per_num = 2 * (int)(config_.periph);

    if(config_.pin_config.tx.port != DSY_GPIOX){
        //check tx against periph
        if(checkPinMatch(&GPIO_InitStruct, config_.pin_config.tx, per_num) == Result::ERR){
            return Result::ERR;
        }

        //setup tx pin
        port                = dsy_hal_map_get_port(&config_.pin_config.tx);
        GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&config_.pin_config.tx);
        tx = GPIO_InitStruct.Pin;
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    }

    if(config_.pin_config.rx.port != DSY_GPIOX){

        //check rx against periph
        if(checkPinMatch(&GPIO_InitStruct, config_.pin_config.rx, per_num + 1) == Result::ERR){
            return Result::ERR;
        }

        //setup rx pin
        port                = dsy_hal_map_get_port(&config_.pin_config.rx);
        GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&config_.pin_config.rx);
        rx = GPIO_InitStruct.Pin;
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    }

    return Result::OK;
}

UartHandler::Result UartHandler::Impl::DeInitPins()
{
    uint16_t      pin;
    port = dsy_hal_map_get_port(&config_.pin_config.tx);
    pin  = dsy_hal_map_get_pin(&config_.pin_config.tx);
    HAL_GPIO_DeInit(port, pin);
    port = dsy_hal_map_get_port(&config_.pin_config.rx);
    pin  = dsy_hal_map_get_pin(&config_.pin_config.rx);
    HAL_GPIO_DeInit(port, pin);

    return Result::OK;
}

// Callbacks
static void UARTRxComplete(UartHandler::Impl* impl)
{
    size_t len, cur_pos;
    //get current write pointer
    cur_pos = (impl->rx_size
               - ((DMA_Stream_TypeDef*)impl->huart1.hdmarx->Instance)->NDTR)
              & (impl->rx_size - 1);
    //calculate how far the DMA write pointer has moved
    len = (cur_pos - impl->rx_last_pos + impl->rx_size) % impl->rx_size;
    //check message size
    if(len <= impl->rx_size)
    {
        impl->dma_fifo_rx->Advance(len);
        impl->rx_last_pos = cur_pos;
#ifdef UART_RX_DOUBLE_BUFFER
        // Copy to queue fifo we don't want to use primary fifo to avoid
        // changes to the buffer while its being processed
        uint8_t processbuf[256];
        impl->dma_fifo_rx->ImmediateRead(processbuf, len);
        impl->queue_rx.Overwrite(processbuf, len);
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
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
    {
        UARTRxComplete(MapInstanceToHandle(huart->Instance));
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
    MapInstanceToHandle(huart->Instance)->rx_active = false;
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

//gotta expand the macros to put them in an arr
void GPIOA_CLK_ENABLE() { __HAL_RCC_GPIOA_CLK_ENABLE(); }
void GPIOB_CLK_ENABLE() { __HAL_RCC_GPIOB_CLK_ENABLE(); }
void GPIOC_CLK_ENABLE() { __HAL_RCC_GPIOC_CLK_ENABLE(); }
void GPIOD_CLK_ENABLE() { __HAL_RCC_GPIOD_CLK_ENABLE(); }
void GPIOE_CLK_ENABLE() { __HAL_RCC_GPIOE_CLK_ENABLE(); }
void GPIOF_CLK_ENABLE() { __HAL_RCC_GPIOF_CLK_ENABLE(); }
void GPIOG_CLK_ENABLE() { __HAL_RCC_GPIOG_CLK_ENABLE(); }
void GPIOH_CLK_ENABLE() { __HAL_RCC_GPIOH_CLK_ENABLE(); }
void GPIOI_CLK_ENABLE() { __HAL_RCC_GPIOI_CLK_ENABLE(); }
void GPIOJ_CLK_ENABLE() { __HAL_RCC_GPIOJ_CLK_ENABLE(); }
void GPIOK_CLK_ENABLE() { __HAL_RCC_GPIOK_CLK_ENABLE(); }

typedef void (*VoidFunc) (void);

void GpioClockEnable(GPIO_TypeDef* port){
    VoidFunc func_p[] = {GPIOA_CLK_ENABLE, GPIOB_CLK_ENABLE, GPIOC_CLK_ENABLE,
                        GPIOD_CLK_ENABLE, GPIOE_CLK_ENABLE, GPIOF_CLK_ENABLE,
                        GPIOG_CLK_ENABLE, GPIOH_CLK_ENABLE, GPIOI_CLK_ENABLE,
                        GPIOJ_CLK_ENABLE, GPIOK_CLK_ENABLE};

    GPIO_TypeDef* ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG,
                            GPIOH, GPIOI, GPIOJ, GPIOK};

    for(int i = 0; i < 11; i++){
        if(port == ports[i]){
            func_p[i]();
            return;
        }
    }
}

//more silly macro expansion
void USART1_CLK_ENABLE(){ __HAL_RCC_USART1_CLK_ENABLE(); };
void USART2_CLK_ENABLE(){ __HAL_RCC_USART2_CLK_ENABLE(); };
void USART3_CLK_ENABLE(){ __HAL_RCC_USART3_CLK_ENABLE(); };
void UART4_CLK_ENABLE(){ __HAL_RCC_UART4_CLK_ENABLE(); };
void UART5_CLK_ENABLE(){ __HAL_RCC_UART5_CLK_ENABLE(); };
void USART6_CLK_ENABLE(){ __HAL_RCC_USART6_CLK_ENABLE(); };
void UART7_CLK_ENABLE(){ __HAL_RCC_UART7_CLK_ENABLE(); };
void UART8_CLK_ENABLE(){ __HAL_RCC_UART8_CLK_ENABLE(); };
void LPUART1_CLK_ENABLE(){ __HAL_RCC_LPUART1_CLK_ENABLE(); };

void UartClockEnable(USART_TypeDef* periph){
    VoidFunc func_p[] = {USART1_CLK_ENABLE, USART2_CLK_ENABLE, USART3_CLK_ENABLE, 
                        UART4_CLK_ENABLE, UART5_CLK_ENABLE, USART6_CLK_ENABLE, 
                        UART7_CLK_ENABLE, UART8_CLK_ENABLE, LPUART1_CLK_ENABLE};
    USART_TypeDef* periphs[] = {USART1, USART2, USART3, UART4, UART5, USART6, UART7, UART8, LPUART1};

    for(int i = 0; i < 9; i++){
        if(periph == periphs[i]){
            func_p[i]();
            return;
        }
    }
}

void EnableNvic(USART_TypeDef* periph){
    IRQn_Type types[] = {USART1_IRQn, USART2_IRQn, USART3_IRQn,
                         UART4_IRQn, UART5_IRQn, USART6_IRQn,
                         UART7_IRQn, UART8_IRQn, LPUART1_IRQn};
    
    USART_TypeDef* periphs[] = {USART1, USART2, USART3, UART4, UART5, USART6, UART7, UART8, LPUART1};

    for(int i = 0; i < 9; i++){
        if(periphs[i] == periph){
            HAL_NVIC_SetPriority(types[i], 0, 0);
            HAL_NVIC_EnableIRQ(types[i]);
            return;
        }
    }
}

// HAL Interface functions
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    UartHandler::Impl* handle = MapInstanceToHandle(uartHandle->Instance);
    GpioClockEnable(handle->port);
    UartClockEnable(handle->periph);
    
    if (handle->InitPins() == UartHandler::Result::ERR){
        Error_Handler();
    }

    /* USART1 DMA Init */
    /* USART1_RX Init */
    //usart1 uses dma by default for now
    if(handle->periph == USART1){
        handle->hdma_usart1_rx.Instance                 = DMA1_Stream5;
        handle->hdma_usart1_rx.Init.Request             = DMA_REQUEST_USART1_RX;
        handle->hdma_usart1_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        handle->hdma_usart1_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
        handle->hdma_usart1_rx.Init.MemInc              = DMA_MINC_ENABLE;
        handle->hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        handle->hdma_usart1_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        handle->hdma_usart1_rx.Init.Mode                = DMA_CIRCULAR;
        handle->hdma_usart1_rx.Init.Priority            = DMA_PRIORITY_LOW;
        handle->hdma_usart1_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
        if(HAL_DMA_Init(&handle->hdma_usart1_rx) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(uartHandle, hdmarx, handle->hdma_usart1_rx);
    }

    /* interrupt Init */
    EnableNvic(handle->periph);

    /* USER CODE BEGIN USART1_MspInit 1 */
    __HAL_UART_ENABLE_IT(&handle->huart1, UART_IT_IDLE);
    // Disable HalfTransfer Interrupt
    ((DMA_Stream_TypeDef*)handle->hdma_usart1_rx.Instance)->CR
        &= ~(DMA_SxCR_HTIE);

    /* USER CODE END USART1_MspInit 1 */   
}

//even sillier macro expansion
void USART1_CLK_DISABLE(){ __HAL_RCC_USART1_CLK_DISABLE(); };
void USART2_CLK_DISABLE(){ __HAL_RCC_USART2_CLK_DISABLE(); };
void USART3_CLK_DISABLE(){ __HAL_RCC_USART3_CLK_DISABLE(); };
void UART4_CLK_DISABLE(){ __HAL_RCC_UART4_CLK_DISABLE(); };
void UART5_CLK_DISABLE(){ __HAL_RCC_UART5_CLK_DISABLE(); };
void USART6_CLK_DISABLE(){ __HAL_RCC_USART6_CLK_DISABLE(); };
void UART7_CLK_DISABLE(){ __HAL_RCC_UART7_CLK_DISABLE(); };
void UART8_CLK_DISABLE(){ __HAL_RCC_UART8_CLK_DISABLE(); };
void LPUART1_CLK_DISABLE(){ __HAL_RCC_LPUART1_CLK_DISABLE(); };

void UartClockDisable(USART_TypeDef* periph){
    VoidFunc func_p[] = {USART1_CLK_DISABLE, USART2_CLK_DISABLE, USART3_CLK_DISABLE, 
                        UART4_CLK_DISABLE, UART5_CLK_DISABLE, USART6_CLK_DISABLE, 
                        UART7_CLK_DISABLE, UART8_CLK_DISABLE, LPUART1_CLK_DISABLE};
    USART_TypeDef* periphs[] = {USART1, USART2, USART3, UART4, UART5, USART6, UART7, UART8, LPUART1};

    for(int i = 0; i < 9; i++){
        if(periph == periphs[i]){
            func_p[i]();
            return;
        }
    }
}

void DisableIrq (USART_TypeDef* periph){
    IRQn_Type types[] = {USART1_IRQn, USART2_IRQn, USART3_IRQn,
                         UART4_IRQn, UART5_IRQn, USART6_IRQn,
                         UART7_IRQn, UART8_IRQn, LPUART1_IRQn};
    
    USART_TypeDef* periphs[] = {USART1, USART2, USART3, UART4, UART5, USART6, UART7, UART8, LPUART1};

    for(int i = 0; i < 9; i++){
        if(periphs[i] == periph){
            HAL_NVIC_DisableIRQ(types[i]);
            return;
        }
    }
}


void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
    UartHandler::Impl* handle = MapInstanceToHandle(uartHandle->Instance);

    UartClockDisable(uartHandle->Instance);
    HAL_GPIO_DeInit(handle->port, handle->tx | handle->rx);
    HAL_DMA_DeInit(uartHandle->hdmarx);
    DisableIrq(uartHandle->Instance);
}

void UART_IRQHandler(UartHandler::Impl* handle){
    HAL_UART_IRQHandler(&handle->huart1);
    //        if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE))
    //        {
    if((handle->huart1.Instance->ISR & UART_FLAG_IDLE) == UART_FLAG_IDLE)
    {
        HAL_UART_RxCpltCallback(&handle->huart1);
        //__HAL_UART_CLEAR_IDLEFLAG(&huart1);
        handle->huart1.Instance->ICR = UART_FLAG_IDLE;
    }
}

// HAL Interrupts.
extern "C"
{
    void USART1_IRQHandler() 
    {
        UART_IRQHandler(&uart_handles[0]);
    }
    void USART2_IRQHandler() 
    {
        UART_IRQHandler(&uart_handles[1]);
    }
    void USART3_IRQHandler() 
    {
        UART_IRQHandler(&uart_handles[2]);
    }
    void UART4_IRQHandler() 
    {
        UART_IRQHandler(&uart_handles[3]);
    }
    void UART5_IRQHandler() 
    {
        UART_IRQHandler(&uart_handles[4]);
    }
    void USART6_IRQHandler() 
    {
        UART_IRQHandler(&uart_handles[5]);
    }
    void UART7_IRQHandler() 
    {
        UART_IRQHandler(&uart_handles[6]);
    }
    void UART8_IRQHandler() 
    {
        UART_IRQHandler(&uart_handles[7]);
    }
    void LPUART1_IRQHandler() 
    {
        UART_IRQHandler(&uart_handles[8]);
    }

    void DMA1_Stream5_IRQHandler()
    {
        //TODO for now USART1 is the only one working with DMA
        //in the future we want to keep track of who connects to which DMA
        //stream, then refer to that info here
        HAL_DMA_IRQHandler(&uart_handles[0].hdma_usart1_rx);
    }

}

// ======================================================================
// UartHandler > UartHandlePimpl
// ======================================================================

UartHandler::Result UartHandler::Init(const Config& config){
    pimpl_ = &uart_handles[int(config.periph)];
    return pimpl_->Init(config);
}

const UartHandler::Config& UartHandler::GetConfig() const{
    return pimpl_->GetConfig();
}

int UartHandler::PollReceive(uint8_t *buff, size_t size, uint32_t timeout){
    return pimpl_->PollReceive(buff, size, timeout);
}

UartHandler::Result UartHandler::StartRx(){
    return pimpl_->StartRx();
}

bool UartHandler::RxActive(){
    return pimpl_->RxActive();
}

UartHandler::Result UartHandler::FlushRx(){
    return pimpl_->FlushRx();
}

UartHandler::Result UartHandler::PollTx(uint8_t *buff, size_t size){
    return pimpl_->PollTx(buff, size);
}

uint8_t UartHandler::PopRx(){
    return pimpl_->PopRx();
}

size_t UartHandler::Readable(){
    return pimpl_->Readable();
}

int UartHandler::CheckError(){
    return pimpl_->CheckError();
}