#include <stm32h7xx_hal.h>
#include "per/uart.h"
#include "util/ringbuffer.h"
#include "util/scopedirqblocker.h"

extern "C"
{
#include "util/hal_map.h"
}

using namespace daisy;

#define UART_RX_BUFF_SIZE 256

// the fifo buffer to be DMA read into
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
    struct UartDmaJob
    {
        uint8_t*                 data_rx          = nullptr;
        uint8_t*                 data_tx          = nullptr;
        uint16_t                 size             = 0;
        StartCallbackFunctionPtr start_callback   = nullptr;
        EndCallbackFunctionPtr   end_callback     = nullptr;
        void*                    callback_context = nullptr;
        DmaDirection             direction        = DmaDirection::TX;

        bool IsValidJob() const
        {
            if(direction == DmaDirection::TX)
            {
                return data_tx != nullptr;
            }
            else if(direction == DmaDirection::RX)
            {
                return data_rx != nullptr;
            }
            return false;
        }
        void Invalidate() { data_rx = data_tx = nullptr; }
    };

    Result Init(const Config& config);

    const Config& GetConfig() const { return config_; }

    Result BlockingTransmit(uint8_t* buff, size_t size, uint32_t timeout);
    Result BlockingReceive(uint8_t* buff, size_t size, uint32_t timeout);

    Result DmaTransmit(uint8_t*                 buff,
                       size_t                   size,
                       StartCallbackFunctionPtr start_callback,
                       EndCallbackFunctionPtr   end_callback,
                       void*                    callback_context);

    Result DmaReceive(uint8_t*                 buff,
                      size_t                   size,
                      StartCallbackFunctionPtr start_callback,
                      EndCallbackFunctionPtr   end_callback,
                      void*                    callback_context);


    Result StartDmaTx(uint8_t*                 buff,
                      size_t                   size,
                      StartCallbackFunctionPtr start_callback,
                      EndCallbackFunctionPtr   end_callback,
                      void*                    callback_context);

    Result StartDmaRx(uint8_t*                 buff,
                      size_t                   size,
                      StartCallbackFunctionPtr start_callback,
                      EndCallbackFunctionPtr   end_callback,
                      void*                    callback_context);

    static void GlobalInit();
    static bool IsDmaBusy();
    static void DmaTransferFinished(UART_HandleTypeDef* huart, Result result);

    static void QueueDmaTransfer(size_t uart_idx, const UartDmaJob& job);
    static bool IsDmaTransferQueuedFor(size_t uart_idx);

    static void DmaReceiveFifoEndCallback(void* context, Result res);

    Result SetDmaPeripheral();

    Result InitDma(bool rx, bool tx);

    Result InitPins();

    Result DeInitPins();

    void HandleFifo();

    Result DmaReceiveFifo();

    Result FlushFifo();

    uint8_t PopFifo();

    size_t ReadableFifo();

    int CheckError();

    static constexpr uint8_t      kNumUartWithDma = 9;
    static volatile int8_t        dma_active_peripheral_;
    static UartDmaJob             queued_dma_transfers_[kNumUartWithDma];
    static EndCallbackFunctionPtr next_end_callback_;
    static void*                  next_callback_context_;

    Config             config_;
    UART_HandleTypeDef huart_;
    DMA_HandleTypeDef  hdma_rx_;
    DMA_HandleTypeDef  hdma_tx_;

    bool            using_fifo_;
    UartRingBuffer* dma_fifo_rx_; // pointer to FIFO DMA reads into
    UartRingBuffer  queue_rx_;    // double buffer ( user reads from )
    size_t          rx_last_pos_;
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

void UartHandler::Impl::GlobalInit()
{
    // init the scheduler queue
    dma_active_peripheral_ = -1;
    for(int per = 0; per < kNumUartWithDma; per++)
        queued_dma_transfers_[per] = UartHandler::Impl::UartDmaJob();
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

    // Fifo stuff
    using_fifo_  = false;
    dma_fifo_rx_ = &uart_dma_fifo;
    dma_fifo_rx_->Init();
    queue_rx_.Init();

    return Result::OK;
}


UartHandler::Result UartHandler::Impl::SetDmaPeripheral()
{
    switch(config_.periph)
    {
        case UartHandler::Config::Peripheral::USART_1:
            hdma_rx_.Init.Request = DMA_REQUEST_USART1_RX;
            hdma_tx_.Init.Request = DMA_REQUEST_USART1_TX;
            break;
        case UartHandler::Config::Peripheral::USART_2:
            hdma_rx_.Init.Request = DMA_REQUEST_USART2_RX;
            hdma_tx_.Init.Request = DMA_REQUEST_USART2_TX;
            break;
        case UartHandler::Config::Peripheral::USART_3:
            hdma_rx_.Init.Request = DMA_REQUEST_USART3_RX;
            hdma_tx_.Init.Request = DMA_REQUEST_USART3_TX;
            break;
        case UartHandler::Config::Peripheral::UART_4:
            hdma_rx_.Init.Request = DMA_REQUEST_UART4_RX;
            hdma_tx_.Init.Request = DMA_REQUEST_UART4_TX;
            break;
        case UartHandler::Config::Peripheral::UART_5:
            hdma_rx_.Init.Request = DMA_REQUEST_UART5_RX;
            hdma_tx_.Init.Request = DMA_REQUEST_UART5_TX;
            break;
        case UartHandler::Config::Peripheral::USART_6:
            hdma_rx_.Init.Request = DMA_REQUEST_USART6_RX;
            hdma_tx_.Init.Request = DMA_REQUEST_USART6_TX;
            break;
        case UartHandler::Config::Peripheral::UART_7:
            hdma_rx_.Init.Request = DMA_REQUEST_UART7_RX;
            hdma_tx_.Init.Request = DMA_REQUEST_UART7_TX;
            break;
        case UartHandler::Config::Peripheral::UART_8:
            hdma_rx_.Init.Request = DMA_REQUEST_UART8_RX;
            hdma_tx_.Init.Request = DMA_REQUEST_UART8_TX;
            break;

        // LPUART1 is on BDMA_REQUEST_LPUART1_RX/TX
        default: return UartHandler::Result::ERR;
    }
    return UartHandler::Result::OK;
}

UartHandler::Result UartHandler::Impl::InitDma(bool rx, bool tx)
{
    hdma_rx_.Instance                 = DMA1_Stream5;
    hdma_rx_.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx_.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx_.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx_.Init.Mode                = using_fifo_ ? DMA_CIRCULAR : DMA_NORMAL;
    hdma_rx_.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    hdma_rx_.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_rx_.Init.Direction           = DMA_PERIPH_TO_MEMORY;

    hdma_tx_.Instance                 = DMA2_Stream4;
    hdma_tx_.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx_.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx_.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx_.Init.Mode                = DMA_NORMAL;
    hdma_tx_.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    hdma_tx_.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_tx_.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    SetDmaPeripheral();

    if(rx)
    {
        if(HAL_DMA_Init(&hdma_rx_) != HAL_OK)
        {
            Error_Handler();
            return UartHandler::Result::ERR;
        }
        __HAL_LINKDMA(&huart_, hdmarx, hdma_rx_);
    }

    if(tx)
    {
        if(HAL_DMA_Init(&hdma_tx_) != HAL_OK)
        {
            Error_Handler();
            return UartHandler::Result::ERR;
        }
        __HAL_LINKDMA(&huart_, hdmatx, hdma_tx_);
    }

    if(using_fifo_)
    {
        // Disable HalfTransfer Interrupt
        ((DMA_Stream_TypeDef*)hdma_rx_.Instance)->CR &= ~(DMA_SxCR_HTIE);

        //enable idle interrupts
        __HAL_UART_ENABLE_IT(&huart_, UART_IT_IDLE);
    }

    return UartHandler::Result::OK;
}

// formerly known as "UARTRxComplete()"
void UartHandler::Impl::HandleFifo()
{
    size_t len, cur_pos;

    //get current write pointer
    cur_pos = (UART_RX_BUFF_SIZE
               - ((DMA_Stream_TypeDef*)huart_.hdmarx->Instance)->NDTR)
              & (UART_RX_BUFF_SIZE - 1);

    //calculate how far the DMA write pointer has moved
    len = (cur_pos - rx_last_pos_ + UART_RX_BUFF_SIZE) % UART_RX_BUFF_SIZE;

    dma_fifo_rx_->Advance(len);
    rx_last_pos_ = cur_pos;

    // Copy to queue fifo we don't want to use primary fifo to avoid
    // changes to the buffer while its being processed
    uint8_t processbuf[256];
    dma_fifo_rx_->ImmediateRead(processbuf, len);
    queue_rx_.Overwrite(processbuf, len);
}

void UartHandler::Impl::DmaTransferFinished(UART_HandleTypeDef* huart,
                                            UartHandler::Result result)
{
    ScopedIrqBlocker block;

    // on an error, reinit the peripheral to clear any flags
    if(result != UartHandler::Result::OK)
        HAL_UART_Init(huart);

    dma_active_peripheral_ = -1;

    if(next_end_callback_ != nullptr)
    {
        // the callback may setup another transmission, hence we shouldn't reset this to
        // nullptr after the callback - it might overwrite the new transmission.
        auto callback      = next_end_callback_;
        next_end_callback_ = nullptr;
        // make the callback
        callback(next_callback_context_, result);
    }

    // the callback could have started a new transmission right away...
    if(IsDmaBusy())
        return;

    // dma is still idle. Check if another UART peripheral waits for a job.
    for(int per = 0; per < kNumUartWithDma; per++)
        if(IsDmaTransferQueuedFor(per))
        {
            UartHandler::Result result;
            if(queued_dma_transfers_[per].direction
               == UartHandler::DmaDirection::TX)
            {
                result = uart_handles[per].StartDmaTx(
                    queued_dma_transfers_[per].data_tx,
                    queued_dma_transfers_[per].size,
                    queued_dma_transfers_[per].start_callback,
                    queued_dma_transfers_[per].end_callback,
                    queued_dma_transfers_[per].callback_context);
            }
            else
            {
                result = uart_handles[per].StartDmaRx(
                    queued_dma_transfers_[per].data_rx,
                    queued_dma_transfers_[per].size,
                    queued_dma_transfers_[per].start_callback,
                    queued_dma_transfers_[per].end_callback,
                    queued_dma_transfers_[per].callback_context);
            }
            if(result == UartHandler::Result::OK)
            {
                // remove the job from the queue
                queued_dma_transfers_[per].Invalidate();
                return;
            }
        }
}

bool UartHandler::Impl::IsDmaBusy()
{
    return dma_active_peripheral_ >= 0;
}

bool UartHandler::Impl::IsDmaTransferQueuedFor(size_t uart_idx)
{
    return queued_dma_transfers_[uart_idx].IsValidJob();
}

void UartHandler::Impl::QueueDmaTransfer(size_t uart_idx, const UartDmaJob& job)
{
    // wait for any previous job on this peripheral to finish
    // and the queue position to become free
    while(IsDmaTransferQueuedFor(uart_idx))
    {
        continue;
    };

    // queue the job
    ScopedIrqBlocker block;
    queued_dma_transfers_[uart_idx] = job;
}


UartHandler::Result UartHandler::Impl::DmaTransmit(
    uint8_t*                              buff,
    size_t                                size,
    UartHandler::StartCallbackFunctionPtr start_callback,
    UartHandler::EndCallbackFunctionPtr   end_callback,
    void*                                 callback_context)
{
    // if dma is currently running - queue a job
    if(IsDmaBusy())
    {
        UartDmaJob job;
        job.data_tx          = buff;
        job.size             = size;
        job.direction        = UartHandler::DmaDirection::TX;
        job.start_callback   = start_callback;
        job.end_callback     = end_callback;
        job.callback_context = callback_context;

        const int uart_idx = int(config_.periph);

        // queue a job (blocks until the queue position is free)
        QueueDmaTransfer(uart_idx, job);
        // TODO: the user can't tell if he got returned "OK"
        // because the transfer was executed or because it was queued...
        // should we change that?
        return UartHandler::Result::OK;
    }

    return StartDmaTx(
        buff, size, start_callback, end_callback, callback_context);
}

UartHandler::Result UartHandler::Impl::StartDmaTx(
    uint8_t*                              buff,
    size_t                                size,
    UartHandler::StartCallbackFunctionPtr start_callback,
    UartHandler::EndCallbackFunctionPtr   end_callback,
    void*                                 callback_context)
{
    while(HAL_UART_GetState(&huart_) != HAL_UART_STATE_READY) {};

    if(InitDma(false, true) != UartHandler::Result::OK)
    {
        if(end_callback)
            end_callback(callback_context, UartHandler::Result::ERR);
        return UartHandler::Result::ERR;
    }

    ScopedIrqBlocker block;

    dma_active_peripheral_ = int(config_.periph);
    next_end_callback_     = end_callback;
    next_callback_context_ = callback_context;

    if(start_callback)
        start_callback(callback_context);

    if(HAL_UART_Transmit_DMA(&huart_, buff, size) != HAL_OK)
    {
        dma_active_peripheral_ = -1;
        next_end_callback_     = NULL;
        next_callback_context_ = NULL;
        if(end_callback)
            end_callback(callback_context, UartHandler::Result::ERR);
        return UartHandler::Result::ERR;
    }
    return UartHandler::Result::OK;
}

UartHandler::Result UartHandler::Impl::DmaReceive(
    uint8_t*                              buff,
    size_t                                size,
    UartHandler::StartCallbackFunctionPtr start_callback,
    UartHandler::EndCallbackFunctionPtr   end_callback,
    void*                                 callback_context)
{
    // if dma is currently running - queue a job
    if(IsDmaBusy())
    {
        UartDmaJob job;
        job.data_rx          = buff;
        job.size             = size;
        job.direction        = UartHandler::DmaDirection::RX;
        job.start_callback   = start_callback;
        job.end_callback     = end_callback;
        job.callback_context = callback_context;

        const int uart_idx = int(config_.periph);

        // queue a job (blocks until the queue position is free)
        QueueDmaTransfer(uart_idx, job);
        // TODO: the user can't tell if he got returned "OK"
        // because the transfer was executed or because it was queued...
        // should we change that?
        return UartHandler::Result::OK;
    }

    return StartDmaRx(
        buff, size, start_callback, end_callback, callback_context);
}

UartHandler::Result UartHandler::Impl::StartDmaRx(
    uint8_t*                              buff,
    size_t                                size,
    UartHandler::StartCallbackFunctionPtr start_callback,
    UartHandler::EndCallbackFunctionPtr   end_callback,
    void*                                 callback_context)
{
    while(HAL_UART_GetState(&huart_) != HAL_UART_STATE_READY) {};

    if(InitDma(true, false) != UartHandler::Result::OK)
    {
        if(end_callback)
            end_callback(callback_context, UartHandler::Result::ERR);
        return UartHandler::Result::ERR;
    }

    ScopedIrqBlocker block;

    dma_active_peripheral_ = int(config_.periph);
    next_end_callback_     = end_callback;
    next_callback_context_ = callback_context;

    if(start_callback)
        start_callback(callback_context);

    if(HAL_UART_Receive_DMA(&huart_, buff, size) != HAL_OK)
    {
        dma_active_peripheral_ = -1;
        next_end_callback_     = NULL;
        next_callback_context_ = NULL;
        if(end_callback)
            end_callback(callback_context, UartHandler::Result::ERR);
        return UartHandler::Result::ERR;
    }
    return UartHandler::Result::OK;
}

UartHandler::Result
UartHandler::Impl::BlockingReceive(uint8_t* buff, size_t size, uint32_t timeout)
{
    if(HAL_UART_Receive(&huart_, (uint8_t*)buff, size, timeout) != HAL_OK)
    {
        return Result::ERR;
    }
    return Result::OK;
}

UartHandler::Result UartHandler::Impl::BlockingTransmit(uint8_t* buff,
                                                        size_t   size,
                                                        uint32_t timeout)
{
    if(HAL_UART_Transmit(&huart_, (uint8_t*)buff, size, timeout) != HAL_OK)
    {
        return Result::ERR;
    }
    return Result::OK;
}

//gets called if the buffer is overrun, transfer buffers and restart DMA
void UartHandler::Impl::DmaReceiveFifoEndCallback(void*               context,
                                                  UartHandler::Result res)
{
    // when the DMA hits the end in circular mode, it's considered an ERROR
    // for some reason it sometimes comes up as OK as well though
    // for now we'll catch the errors and just reset
    if(res == UartHandler::Result::OK || res == UartHandler::Result::ERR)
    {
        UartHandler::Impl* handle = (UartHandler::Impl*)context;
        handle->HandleFifo();
        HAL_UART_Init(&handle->huart_);
        handle->DmaReceiveFifo();
    }
}

UartHandler::Result UartHandler::Impl::DmaReceiveFifo()
{
    using_fifo_ = true;
    return DmaReceive((uint8_t*)dma_fifo_rx_->GetMutableBuffer(),
                      UART_RX_BUFF_SIZE,
                      NULL,
                      UartHandler::Impl::DmaReceiveFifoEndCallback,
                      (void*)this);
}

UartHandler::Result UartHandler::Impl::FlushFifo()
{
    queue_rx_.Flush();
    return Result::OK;
}

uint8_t UartHandler::Impl::PopFifo()
{
    return queue_rx_.Read();
}

size_t UartHandler::Impl::ReadableFifo()
{
    return queue_rx_.readable();
}

int UartHandler::Impl::CheckError()
{
    return HAL_UART_GetError(&huart_);
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
                           {{DSY_GPIOA, 0}, GPIO_AF8_UART4}};
pin_alt uart4_pins_rx[] = {{{DSY_GPIOB, 8}, GPIO_AF8_UART4},
                           {{DSY_GPIOC, 11}, GPIO_AF8_UART4},
                           {{DSY_GPIOA, 1}, GPIO_AF8_UART4}};

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

volatile int8_t UartHandler::Impl::dma_active_peripheral_;
UartHandler::Impl::UartDmaJob
    UartHandler::Impl::queued_dma_transfers_[kNumUartWithDma];

UartHandler::EndCallbackFunctionPtr UartHandler::Impl::next_end_callback_;
void*                               UartHandler::Impl::next_callback_context_;

// HAL Interface functions
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    UartHandler::Impl* handle = MapInstanceToHandle(uartHandle->Instance);
    dsy_hal_map_gpio_clk_enable(handle->config_.pin_config.rx.port);
    dsy_hal_map_gpio_clk_enable(handle->config_.pin_config.tx.port);

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

    // enable the interrupts
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
    HAL_DMA_DeInit(uartHandle->hdmatx);

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

extern "C" void dsy_uart_global_init()
{
    UartHandler::Impl::GlobalInit();
}

// HAL Interrupts.
void UART_IRQHandler(UartHandler::Impl* handle)
{
    HAL_UART_IRQHandler(&handle->huart_);

    if(__HAL_UART_GET_FLAG(&handle->huart_, UART_FLAG_IDLE)
       && handle->using_fifo_)
    {
        handle->HandleFifo();
        handle->huart_.Instance->ICR = UART_FLAG_IDLE;
    }
}

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
}

void HalUartDmaRxStreamCallback(void)
{
    ScopedIrqBlocker block;
    if(UartHandler::Impl::dma_active_peripheral_ >= 0)
        HAL_DMA_IRQHandler(
            &uart_handles[UartHandler::Impl::dma_active_peripheral_].hdma_rx_);
}
extern "C" void DMA1_Stream5_IRQHandler(void)
{
    HalUartDmaRxStreamCallback();
}

void HalUartDmaTxStreamCallback(void)
{
    ScopedIrqBlocker block;
    if(UartHandler::Impl::dma_active_peripheral_ >= 0)
        HAL_DMA_IRQHandler(
            &uart_handles[UartHandler::Impl::dma_active_peripheral_].hdma_tx_);
}
extern "C" void DMA2_Stream4_IRQHandler(void)
{
    HalUartDmaTxStreamCallback();
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    UartHandler::Impl::DmaTransferFinished(huart, UartHandler::Result::OK);
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    UartHandler::Impl::DmaTransferFinished(huart, UartHandler::Result::OK);
}

extern "C" void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart)
{
    UartHandler::Impl::DmaTransferFinished(huart, UartHandler::Result::ERR);
}

extern "C" void HAL_UART_AbortCpltCallback(UART_HandleTypeDef* huart)
{
    //    asm("bkpt 255");
}

extern "C" void HAL_UART_AbortTransmitCpltCallback(UART_HandleTypeDef* huart)
{
    //    asm("bkpt 255");
}

extern "C" void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef* huart)
{
    //    asm("bkpt 255");
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

UartHandler::Result
UartHandler::BlockingTransmit(uint8_t* buff, size_t size, uint32_t timeout)
{
    return pimpl_->BlockingTransmit(buff, size, timeout);
}

UartHandler::Result
UartHandler::BlockingReceive(uint8_t* buffer, uint16_t size, uint32_t timeout)
{
    return pimpl_->BlockingReceive(buffer, size, timeout);
}


UartHandler::Result
UartHandler::DmaTransmit(uint8_t*                              buff,
                         size_t                                size,
                         UartHandler::StartCallbackFunctionPtr start_callback,
                         UartHandler::EndCallbackFunctionPtr   end_callback,
                         void*                                 callback_context)
{
    return pimpl_->DmaTransmit(
        buff, size, start_callback, end_callback, callback_context);
}

UartHandler::Result
UartHandler::DmaReceive(uint8_t*                              buff,
                        size_t                                size,
                        UartHandler::StartCallbackFunctionPtr start_callback,
                        UartHandler::EndCallbackFunctionPtr   end_callback,
                        void*                                 callback_context)
{
    return pimpl_->DmaReceive(
        buff, size, start_callback, end_callback, callback_context);
}

UartHandler::Result UartHandler::DmaReceiveFifo()
{
    return pimpl_->DmaReceiveFifo();
}

UartHandler::Result UartHandler::FlushFifo()
{
    return pimpl_->FlushFifo();
}

uint8_t UartHandler::PopFifo()
{
    return pimpl_->PopFifo();
}

size_t UartHandler::ReadableFifo()
{
    return pimpl_->ReadableFifo();
}

int UartHandler::CheckError()
{
    return pimpl_->CheckError();
}

// ========= wrappers to be deprecated  =========

int UartHandler::PollReceive(uint8_t* buff, size_t size, uint32_t timeout)
{
    return pimpl_->BlockingReceive(buff, size, timeout) == Result::ERR;
}

UartHandler::Result UartHandler::PollTx(uint8_t* buff, size_t size)
{
    return pimpl_->BlockingTransmit(buff, size, 10);
}

uint8_t UartHandler::PopRx()
{
    return pimpl_->PopFifo();
}

UartHandler::Result UartHandler::StartRx()
{
    return pimpl_->DmaReceiveFifo();
}

UartHandler::Result UartHandler::FlushRx()
{
    return pimpl_->FlushFifo();
}

size_t UartHandler::Readable()
{
    return pimpl_->ReadableFifo();
}
