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
    while(1) {}
}

class UartHandler::Impl
{
  public:
    UartHandler::Result Init(const UartHandler::Config& config);

    const UartHandler::Config& GetConfig() const { return config_; }

    int PollReceive(uint8_t* buff, size_t size, uint32_t timeout);

    UartHandler::Result StartRx();

    bool RxActive();

    UartHandler::Result FlushRx();

    UartHandler::Result PollTx(uint8_t* buff, size_t size);

    uint8_t PopRx();

    size_t Readable();

    int CheckError();

    UartHandler::Result InitPins();

    UartHandler::Result DeInitPins();

    void UARTRxComplete();

    UART_HandleTypeDef huart_;
    DMA_HandleTypeDef  hdma_rx_;
    bool               receiving_;
    size_t             rx_size_, rx_last_pos_;
    UartRingBuffer*    dma_fifo_rx_;
    bool               rx_active_, tx_active_;
#ifdef UART_RX_DOUBLE_BUFFER
    UartRingBuffer queue_rx_;
#endif

    UartHandler::Config config_;
};


// ================================================================
// Global references for the availabel UartHandler::Impl(s)
// ================================================================

static UartHandler::Impl uart_handles[9];

UartHandler::Impl* MapInstanceToHandle(USART_TypeDef* instance)
{
    constexpr USART_TypeDef* instances[9] = {USART1,
                                             USART2,
                                             USART3,
                                             UART4,
                                             UART5,
                                             USART6,
                                             UART7,
                                             UART8,
                                             LPUART1}; // map HAL instances
    for(int i = 0; i < 9; i++)
    {
        if(instance == instances[i])
        {
            return &uart_handles[i];
        }
    }

    /* error */
    return NULL;
}

UartHandler::Result UartHandler::Impl::Init(const UartHandler::Config& config)
{
    config_ = config;

    USART_TypeDef* periph;
    switch(config_.periph)
    {
        case Config::Peripheral::USART_1: periph = USART1; break;
        case Config::Peripheral::USART_2: periph = USART2; break;
        case Config::Peripheral::USART_3: periph = USART3; break;
        case Config::Peripheral::UART_4: periph = UART4; break;
        case Config::Peripheral::UART_5: periph = UART5; break;
        case Config::Peripheral::USART_6: periph = USART6; break;
        case Config::Peripheral::UART_7: periph = UART7; break;
        case Config::Peripheral::UART_8: periph = UART8; break;
        case Config::Peripheral::LPUART_1: periph = LPUART1; break;
        default: return Result::ERR;
    }

    uint32_t parity;
    switch(config_.parity)
    {
        case Config::Parity::NONE: parity = UART_PARITY_NONE; break;
        case Config::Parity::EVEN: parity = UART_PARITY_EVEN; break;
        case Config::Parity::ODD: parity = UART_PARITY_ODD; break;
        default: return Result::ERR;
    }

    uint32_t stop_bits;
    switch(config_.stopbits)
    {
        case Config::StopBits::BITS_0_5: stop_bits = UART_STOPBITS_0_5; break;
        case Config::StopBits::BITS_1: stop_bits = UART_STOPBITS_1; break;
        case Config::StopBits::BITS_1_5: stop_bits = UART_STOPBITS_1_5; break;
        case Config::StopBits::BITS_2: stop_bits = UART_STOPBITS_2; break;
        default: return Result::ERR;
    }

    uint32_t mode;
    switch(config_.mode)
    {
        case Config::Mode::RX: mode = UART_MODE_RX; break;
        case Config::Mode::TX: mode = UART_MODE_TX; break;
        case Config::Mode::TX_RX: mode = UART_MODE_TX_RX; break;
        default: return Result::ERR;
    }

    uint32_t wordlen;
    switch(config_.wordlength)
    {
        case Config::WordLength::BITS_7: wordlen = UART_WORDLENGTH_7B; break;
        case Config::WordLength::BITS_8: wordlen = UART_WORDLENGTH_8B; break;
        case Config::WordLength::BITS_9: wordlen = UART_WORDLENGTH_9B; break;
        default: return Result::ERR;
    }

    huart_.Instance                    = periph;
    huart_.Init.BaudRate               = config.baudrate;
    huart_.Init.WordLength             = wordlen;
    huart_.Init.StopBits               = stop_bits;
    huart_.Init.Parity                 = parity;
    huart_.Init.Mode                   = mode;
    huart_.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart_.Init.OverSampling           = UART_OVERSAMPLING_16;
    huart_.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart_.Init.ClockPrescaler         = UART_PRESCALER_DIV1;
    huart_.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if(HAL_UART_Init(&huart_) != HAL_OK)
    {
        return Result::ERR;
    }
    if(HAL_UARTEx_SetTxFifoThreshold(&huart_, UART_TXFIFO_THRESHOLD_1_8)
       != HAL_OK)
    {
        return Result::ERR;
    }
    if(HAL_UARTEx_SetRxFifoThreshold(&huart_, UART_RXFIFO_THRESHOLD_1_8)
       != HAL_OK)
    {
        return Result::ERR;
    }
    if(HAL_UARTEx_DisableFifoMode(&huart_) != HAL_OK)
    {
        return Result::ERR;
    }

    // Internal bits
    dma_fifo_rx_ = &uart_dma_fifo;
    dma_fifo_rx_->Init();
    rx_size_ = UART_RX_BUFF_SIZE;
    // Buffer that gets copied
    rx_active_ = false;
    tx_active_ = false;
#ifdef UART_RX_DOUBLE_BUFFER
    queue_rx_.Init();
#endif

    return Result::OK;
}

int UartHandler::Impl::PollReceive(uint8_t* buff, size_t size, uint32_t timeout)
{
    return HAL_UART_Receive(&huart_, (uint8_t*)buff, size, timeout);
}

UartHandler::Result UartHandler::Impl::StartRx()
{
    int status = 0;
    // Now start Rx
    status = HAL_UART_Receive_DMA(
        &huart_, (uint8_t*)dma_fifo_rx_->GetMutableBuffer(), rx_size_);
    if(status == 0)
        rx_active_ = true;
    return rx_active_ ? Result::OK : Result::ERR;
}

bool UartHandler::Impl::RxActive()
{
    return rx_active_;
}

//this originally had a useless status var hanging about
//I don't think we can actually error check this...
UartHandler::Result UartHandler::Impl::FlushRx()
{
#ifdef UART_RX_DOUBLE_BUFFER
    queue_rx_.Flush();
#else
    dma_fifo_rx_->Flush();
#endif
    return Result::OK;
}

UartHandler::Result UartHandler::Impl::PollTx(uint8_t* buff, size_t size)
{
    HAL_StatusTypeDef status
        = HAL_UART_Transmit(&huart_, (uint8_t*)buff, size, 10);
    return (status == HAL_OK ? Result::OK : Result::ERR);
}

int UartHandler::Impl::CheckError()
{
    return HAL_UART_GetError(&huart_);
}

uint8_t UartHandler::Impl::PopRx()
{
#ifdef UART_RX_DOUBLE_BUFFER
    return queue_rx_.Read();
#else
    return dma_fifo_rx_->Read();
#endif
}

size_t UartHandler::Impl::Readable()
{
#ifdef UART_RX_DOUBLE_BUFFER
    return queue_rx_.readable();
#else
    return dma_fifo_rx_->readable();
#endif
}

typedef struct
{
    dsy_gpio_pin pin;
    uint8_t      alt;
} pin_alt;

pin_alt pins_none = {{DSY_GPIOX, 0}, 255};

//valid pins per periph, and the alt they're on
pin_alt usart1_pins_tx[] = {{{DSY_GPIOB, 6}, GPIO_AF7_USART1},
                            {{DSY_GPIOB, 14}, GPIO_AF4_USART1},
                            pins_none};
pin_alt usart1_pins_rx[] = {{{DSY_GPIOB, 7}, GPIO_AF7_USART1},
                            {{DSY_GPIOB, 15}, GPIO_AF4_USART1},
                            pins_none};

pin_alt usart2_pins_tx[]
    = {{{DSY_GPIOA, 2}, GPIO_AF7_USART2}, pins_none, pins_none};
pin_alt usart2_pins_rx[]
    = {{{DSY_GPIOA, 3}, GPIO_AF7_USART2}, pins_none, pins_none};

pin_alt usart3_pins_tx[]
    = {{{DSY_GPIOC, 10}, GPIO_AF7_USART3}, pins_none, pins_none};
pin_alt usart3_pins_rx[]
    = {{{DSY_GPIOC, 11}, GPIO_AF7_USART3}, pins_none, pins_none};

pin_alt uart4_pins_tx[] = {{{DSY_GPIOB, 9}, GPIO_AF8_UART4},
                           {{DSY_GPIOC, 10}, GPIO_AF8_UART4},
                           pins_none};
pin_alt uart4_pins_rx[] = {{{DSY_GPIOB, 8}, GPIO_AF8_UART4},
                           {{DSY_GPIOC, 11}, GPIO_AF8_UART4},
                           pins_none};

pin_alt uart5_pins_tx[] = {{{DSY_GPIOC, 12}, GPIO_AF8_UART5},
                           {{DSY_GPIOB, 6}, GPIO_AF14_UART5},
                           pins_none};
pin_alt uart5_pins_rx[] = {{{DSY_GPIOB, 12}, GPIO_AF14_UART5},
                           {{DSY_GPIOD, 2}, GPIO_AF8_UART5},
                           {{DSY_GPIOB, 5}, GPIO_AF14_UART5}};

pin_alt usart6_pins_tx[] = {pins_none, pins_none, pins_none};
pin_alt usart6_pins_rx[]
    = {{{DSY_GPIOG, 9}, GPIO_AF7_USART6}, pins_none, pins_none};

pin_alt uart7_pins_tx[]
    = {{{DSY_GPIOB, 4}, GPIO_AF11_UART7}, pins_none, pins_none};
pin_alt uart7_pins_rx[] = {pins_none, pins_none, pins_none};

pin_alt uart8_pins_tx[] = {pins_none, pins_none, pins_none};
pin_alt uart8_pins_rx[] = {pins_none, pins_none, pins_none};

pin_alt lpuart1_pins_tx[]
    = {{{DSY_GPIOB, 6}, GPIO_AF8_LPUART}, pins_none, pins_none};
pin_alt lpuart1_pins_rx[]
    = {{{DSY_GPIOB, 7}, GPIO_AF8_LPUART}, pins_none, pins_none};

//an array to hold everything
pin_alt* pins_periphs[] = {usart1_pins_tx,
                           usart1_pins_rx,
                           usart2_pins_tx,
                           usart2_pins_rx,
                           usart3_pins_tx,
                           usart3_pins_rx,
                           uart4_pins_tx,
                           uart4_pins_rx,
                           uart5_pins_tx,
                           uart5_pins_rx,
                           usart6_pins_tx,
                           usart6_pins_rx,
                           uart7_pins_tx,
                           uart7_pins_rx,
                           uart8_pins_tx,
                           uart8_pins_rx,
                           lpuart1_pins_tx,
                           lpuart1_pins_rx};

UartHandler::Result
checkPinMatch(GPIO_InitTypeDef* init, dsy_gpio_pin pin, int p_num)
{
    for(int i = 0; i < 3; i++)
    {
        if(dsy_pin_cmp(&pins_periphs[p_num][i].pin, &pins_none.pin))
        {
            /* skip */
        }

        else if(dsy_pin_cmp(&pins_periphs[p_num][i].pin, &pin))
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

    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    int per_num = 2 * (int)(config_.periph);

    if(config_.pin_config.tx.port != DSY_GPIOX)
    {
        //check tx against periph
        if(checkPinMatch(&GPIO_InitStruct, config_.pin_config.tx, per_num)
           == Result::ERR)
        {
            return Result::ERR;
        }

        //setup tx pin
        GPIO_TypeDef* port  = dsy_hal_map_get_port(&config_.pin_config.tx);
        GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&config_.pin_config.tx);
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    }

    if(config_.pin_config.rx.port != DSY_GPIOX)
    {
        //check rx against periph
        if(checkPinMatch(&GPIO_InitStruct, config_.pin_config.rx, per_num + 1)
           == Result::ERR)
        {
            return Result::ERR;
        }

        //setup rx pin
        GPIO_TypeDef* port  = dsy_hal_map_get_port(&config_.pin_config.rx);
        GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&config_.pin_config.rx);
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    }

    return Result::OK;
}

UartHandler::Result UartHandler::Impl::DeInitPins()
{
    GPIO_TypeDef* port = dsy_hal_map_get_port(&config_.pin_config.tx);
    uint16_t      pin  = dsy_hal_map_get_pin(&config_.pin_config.tx);
    HAL_GPIO_DeInit(port, pin);

    port = dsy_hal_map_get_port(&config_.pin_config.rx);
    pin  = dsy_hal_map_get_pin(&config_.pin_config.rx);
    HAL_GPIO_DeInit(port, pin);

    return Result::OK;
}

// Callbacks
void UartHandler::Impl::UARTRxComplete()
{
    size_t len, cur_pos;
    //get current write pointer
    cur_pos = (rx_size_ - ((DMA_Stream_TypeDef*)huart_.hdmarx->Instance)->NDTR)
              & (rx_size_ - 1);
    //calculate how far the DMA write pointer has moved
    len = (cur_pos - rx_last_pos_ + rx_size_) % rx_size_;
    //check message size
    if(len <= rx_size_)
    {
        dma_fifo_rx_->Advance(len);
        rx_last_pos_ = cur_pos;
#ifdef UART_RX_DOUBLE_BUFFER
        // Copy to queue fifo we don't want to use primary fifo to avoid
        // changes to the buffer while its being processed
        uint8_t processbuf[256];
        dma_fifo_rx_->ImmediateRead(processbuf, len);
        queue_rx_.Overwrite(processbuf, len);
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
    UartHandler::Impl* handle = MapInstanceToHandle(huart->Instance);
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
    {
        handle->UARTRxComplete();
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
    MapInstanceToHandle(huart->Instance)->rx_active_ = false;
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
    UartHandler::Impl* handle = MapInstanceToHandle(uartHandle->Instance);
    dsy_hal_map_gpio_clk_enable(handle->config_.pin_config.rx.port);
    dsy_hal_map_gpio_clk_enable(handle->config_.pin_config.tx.port);

    //enable the clock for our periph
    switch(handle->config_.periph)
    {
        case UartHandler::Config::Peripheral::USART_1:
            __HAL_RCC_USART1_CLK_ENABLE();
            break;
        case UartHandler::Config::Peripheral::USART_2:
            __HAL_RCC_USART2_CLK_ENABLE();
            break;
        case UartHandler::Config::Peripheral::USART_3:
            __HAL_RCC_USART3_CLK_ENABLE();
            break;
        case UartHandler::Config::Peripheral::UART_4:
            __HAL_RCC_UART4_CLK_ENABLE();
            break;
        case UartHandler::Config::Peripheral::UART_5:
            __HAL_RCC_UART5_CLK_ENABLE();
            break;
        case UartHandler::Config::Peripheral::USART_6:
            __HAL_RCC_USART6_CLK_ENABLE();
            break;
        case UartHandler::Config::Peripheral::UART_7:
            __HAL_RCC_UART7_CLK_ENABLE();
            break;
        case UartHandler::Config::Peripheral::UART_8:
            __HAL_RCC_UART8_CLK_ENABLE();
            break;
        case UartHandler::Config::Peripheral::LPUART_1:
            __HAL_RCC_LPUART1_CLK_ENABLE();
            break;
    }

    if(handle->InitPins() == UartHandler::Result::ERR)
    {
        Error_Handler();
    }

    /* USART1 DMA Init */
    /* USART1_RX Init */
    //usart1 uses dma by default for now
    if(handle->huart_.Instance == USART1)
    {
        handle->hdma_rx_.Instance                 = DMA1_Stream5;
        handle->hdma_rx_.Init.Request             = DMA_REQUEST_USART1_RX;
        handle->hdma_rx_.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        handle->hdma_rx_.Init.PeriphInc           = DMA_PINC_DISABLE;
        handle->hdma_rx_.Init.MemInc              = DMA_MINC_ENABLE;
        handle->hdma_rx_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        handle->hdma_rx_.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        handle->hdma_rx_.Init.Mode                = DMA_CIRCULAR;
        handle->hdma_rx_.Init.Priority            = DMA_PRIORITY_LOW;
        handle->hdma_rx_.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
        if(HAL_DMA_Init(&handle->hdma_rx_) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(uartHandle, hdmarx, handle->hdma_rx_);

        // Disable HalfTransfer Interrupt
        ((DMA_Stream_TypeDef*)handle->hdma_rx_.Instance)->CR
            &= ~(DMA_SxCR_HTIE);

        IRQn_Type types[] = {USART1_IRQn,
                             USART2_IRQn,
                             USART3_IRQn,
                             UART4_IRQn,
                             UART5_IRQn,
                             USART6_IRQn,
                             UART7_IRQn,
                             UART8_IRQn,
                             LPUART1_IRQn};

        HAL_NVIC_SetPriority(types[(int)handle->config_.periph], 0, 0);
        HAL_NVIC_EnableIRQ(types[(int)handle->config_.periph]);
    }

    /* USER CODE BEGIN USART1_MspInit 1 */
    __HAL_UART_ENABLE_IT(&handle->huart_, UART_IT_IDLE);
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
    UartHandler::Impl* handle = MapInstanceToHandle(uartHandle->Instance);

    //disable the clock for our periph
    switch(handle->config_.periph)
    {
        case UartHandler::Config::Peripheral::USART_1:
            __HAL_RCC_USART1_CLK_DISABLE();
            break;
        case UartHandler::Config::Peripheral::USART_2:
            __HAL_RCC_USART2_CLK_DISABLE();
            break;
        case UartHandler::Config::Peripheral::USART_3:
            __HAL_RCC_USART3_CLK_DISABLE();
            break;
        case UartHandler::Config::Peripheral::UART_4:
            __HAL_RCC_UART4_CLK_DISABLE();
            break;
        case UartHandler::Config::Peripheral::UART_5:
            __HAL_RCC_UART5_CLK_DISABLE();
            break;
        case UartHandler::Config::Peripheral::USART_6:
            __HAL_RCC_USART6_CLK_DISABLE();
            break;
        case UartHandler::Config::Peripheral::UART_7:
            __HAL_RCC_UART7_CLK_DISABLE();
            break;
        case UartHandler::Config::Peripheral::UART_8:
            __HAL_RCC_UART8_CLK_DISABLE();
            break;
        case UartHandler::Config::Peripheral::LPUART_1:
            __HAL_RCC_LPUART1_CLK_DISABLE();
            break;
    }

    handle->DeInitPins();

    HAL_DMA_DeInit(uartHandle->hdmarx);

    IRQn_Type types[] = {USART1_IRQn,
                         USART2_IRQn,
                         USART3_IRQn,
                         UART4_IRQn,
                         UART5_IRQn,
                         USART6_IRQn,
                         UART7_IRQn,
                         UART8_IRQn,
                         LPUART1_IRQn};


    HAL_NVIC_DisableIRQ(types[(int)handle->config_.periph]);
}

void UART_IRQHandler(UartHandler::Impl* handle)
{
    HAL_UART_IRQHandler(&handle->huart_);
    //        if(__HAL_UART_GET_FLAG(&huart, UART_FLAG_IDLE))
    //        {
    if((handle->huart_.Instance->ISR & UART_FLAG_IDLE) == UART_FLAG_IDLE)
    {
        HAL_UART_RxCpltCallback(&handle->huart_);
        //__HAL_UART_CLEAR_IDLEFLAG(&huart);
        handle->huart_.Instance->ICR = UART_FLAG_IDLE;
    }
}

// HAL Interrupts.
extern "C"
{
    void USART1_IRQHandler() { UART_IRQHandler(&uart_handles[0]); }
    void USART2_IRQHandler() { UART_IRQHandler(&uart_handles[1]); }
    void USART3_IRQHandler() { UART_IRQHandler(&uart_handles[2]); }
    void UART4_IRQHandler() { UART_IRQHandler(&uart_handles[3]); }
    void UART5_IRQHandler() { UART_IRQHandler(&uart_handles[4]); }
    void USART6_IRQHandler() { UART_IRQHandler(&uart_handles[5]); }
    void UART7_IRQHandler() { UART_IRQHandler(&uart_handles[6]); }
    void UART8_IRQHandler() { UART_IRQHandler(&uart_handles[7]); }
    void LPUART1_IRQHandler() { UART_IRQHandler(&uart_handles[8]); }

    void DMA1_Stream5_IRQHandler()
    {
        //TODO for now USART1 is the only one working with DMA
        //in the future we want to keep track of who connects to which DMA
        //stream, then refer to that info here
        HAL_DMA_IRQHandler(&uart_handles[0].hdma_rx_);
    }
}

// ======================================================================
// UartHandler > UartHandlePimpl
// ======================================================================

UartHandler::Result UartHandler::Init(const Config& config)
{
    pimpl_ = &uart_handles[int(config.periph)];
    return pimpl_->Init(config);
}

const UartHandler::Config& UartHandler::GetConfig() const
{
    return pimpl_->GetConfig();
}

int UartHandler::PollReceive(uint8_t* buff, size_t size, uint32_t timeout)
{
    return pimpl_->PollReceive(buff, size, timeout);
}

UartHandler::Result UartHandler::StartRx()
{
    return pimpl_->StartRx();
}

bool UartHandler::RxActive()
{
    return pimpl_->RxActive();
}

UartHandler::Result UartHandler::FlushRx()
{
    return pimpl_->FlushRx();
}

UartHandler::Result UartHandler::PollTx(uint8_t* buff, size_t size)
{
    return pimpl_->PollTx(buff, size);
}

uint8_t UartHandler::PopRx()
{
    return pimpl_->PopRx();
}

size_t UartHandler::Readable()
{
    return pimpl_->Readable();
}

int UartHandler::CheckError()
{
    return pimpl_->CheckError();
}