#include "per/spi.h"
#include "util/scopedirqblocker.h"

extern "C"
{
#include "util/hal_map.h"
}

// TODO
// - fix up rest of lib so that we can add a spi_handle map to the hal map
// - Add configuration for standard spi stuff.


using namespace daisy;

static void Error_Handler()
{
    asm("bkpt 255");
    while(1) {}
}

class SpiHandle::Impl
{
  public:
    struct SpiDmaJob
    {
        uint8_t*                            data_rx          = nullptr;
        uint8_t*                            data_tx          = nullptr;
        uint16_t                            size             = 0;
        SpiHandle::StartCallbackFunctionPtr start_callback   = nullptr;
        SpiHandle::EndCallbackFunctionPtr   end_callback     = nullptr;
        void*                               callback_context = nullptr;
        SpiHandle::DmaDirection direction = SpiHandle::DmaDirection::TX;

        bool IsValidJob() const
        {
            return data_rx != nullptr && data_tx != nullptr;
        }
        void Invalidate() { data_rx = data_tx = nullptr; }
    };
    Result Init(const Config& config);

    const SpiHandle::Config& GetConfig() const { return config_; }
    int                      CheckError();

    Result BlockingTransmit(uint8_t* buff, size_t size, uint32_t timeout);
    Result BlockingReceive(uint8_t* buffer, uint16_t size, uint32_t timeout);
    Result BlockingTransmitAndReceive(uint8_t* tx_buff,
                                      uint8_t* rx_buff,
                                      size_t   size,
                                      uint32_t timeout);
    Result DmaTransmit(uint8_t*                            buff,
                       size_t                              size,
                       SpiHandle::StartCallbackFunctionPtr start_callback,
                       SpiHandle::EndCallbackFunctionPtr   end_callback,
                       void*                               callback_context);
    Result DmaReceive(uint8_t*                            buff,
                      size_t                              size,
                      SpiHandle::StartCallbackFunctionPtr start_callback,
                      SpiHandle::EndCallbackFunctionPtr   end_callback,
                      void*                               callback_context);
    Result
    DmaTransmitAndReceive(uint8_t*                            tx_buff,
                          uint8_t*                            rx_buff,
                          size_t                              size,
                          SpiHandle::StartCallbackFunctionPtr start_callback,
                          SpiHandle::EndCallbackFunctionPtr   end_callback,
                          void*                               callback_context);


    Result InitPins();
    Result DeInitPins();

    Result StartDmaTx(uint8_t*                            buff,
                      size_t                              size,
                      SpiHandle::StartCallbackFunctionPtr start_callback,
                      SpiHandle::EndCallbackFunctionPtr   end_callback,
                      void*                               callback_context);

    Result StartDmaRx(uint8_t*                            buff,
                      size_t                              size,
                      SpiHandle::StartCallbackFunctionPtr start_callback,
                      SpiHandle::EndCallbackFunctionPtr   end_callback,
                      void*                               callback_context);
    Result StartDmaRxTx(uint8_t*                            tx_buff,
                        uint8_t*                            rx_buff,
                        size_t                              size,
                        SpiHandle::StartCallbackFunctionPtr start_callback,
                        SpiHandle::EndCallbackFunctionPtr   end_callback,
                        void*                               callback_context);

    static void GlobalInit();
    static bool IsDmaBusy();
    static void DmaTransferFinished(SPI_HandleTypeDef* hspi,
                                    SpiHandle::Result  result);

    static void QueueDmaTransfer(size_t spi_idx, const SpiDmaJob& job);
    static bool IsDmaTransferQueuedFor(size_t spi_idx);

    Result SetDmaPeripheral();
    Result InitDma();

    static constexpr uint8_t kNumSpiWithDma = 4;
    static volatile int8_t   dma_active_peripheral_;
    static SpiDmaJob         queued_dma_transfers_[kNumSpiWithDma];
    static SpiHandle::EndCallbackFunctionPtr next_end_callback_;
    static void*                             next_callback_context_;

    SpiHandle::Config config_;
    SPI_HandleTypeDef hspi_;
    DMA_HandleTypeDef hdma_spi_rx_;
    DMA_HandleTypeDef hdma_spi_tx_;
};

// ================================================================
// Global references for the available SpiHandle::Impl(s)
// ================================================================

static SpiHandle::Impl spi_handles[6];

SpiHandle::Impl* MapInstanceToHandle(SPI_TypeDef* instance)
{
    // map HAL instances
    constexpr SPI_TypeDef* instances[6] = {SPI1, SPI2, SPI3, SPI4, SPI5, SPI6};

    for(int i = 0; i < 6; i++)
    {
        if(instance == instances[i])
        {
            return &spi_handles[i];
        }
    }

    /* error */
    return NULL;
}

void SpiHandle::Impl::GlobalInit()
{
    // init the scheduler queue
    dma_active_peripheral_ = -1;
    for(int per = 0; per < kNumSpiWithDma; per++)
        queued_dma_transfers_[per] = SpiHandle::Impl::SpiDmaJob();
}

SpiHandle::Result SpiHandle::Impl::Init(const Config& config)
{
    config_ = config;

    SPI_TypeDef* periph;
    switch(config_.periph)
    {
        case Config::Peripheral::SPI_1: periph = SPI1; break;
        case Config::Peripheral::SPI_2: periph = SPI2; break;
        case Config::Peripheral::SPI_3: periph = SPI3; break;
        case Config::Peripheral::SPI_4: periph = SPI4; break;
        case Config::Peripheral::SPI_5: periph = SPI5; break;
        case Config::Peripheral::SPI_6: periph = SPI6; break;
        default: return Result::ERR;
    }

    uint32_t mode;
    switch(config_.mode)
    {
        case Config::Mode::MASTER: mode = SPI_MODE_MASTER; break;
        case Config::Mode::SLAVE: mode = SPI_MODE_SLAVE; break;
        default: return Result::ERR;
    }

    uint32_t direction;
    switch(config_.direction)
    {
        case Config::Direction::TWO_LINES:
            direction = SPI_DIRECTION_2LINES;
            break;
        case Config::Direction::TWO_LINES_TX_ONLY:
            direction = SPI_DIRECTION_2LINES_TXONLY;
            break;
        case Config::Direction::TWO_LINES_RX_ONLY:
            direction = SPI_DIRECTION_2LINES_RXONLY;
            break;
        case Config::Direction::ONE_LINE:
            direction = SPI_DIRECTION_1LINE;
            break;
        default: return Result::ERR;
    }

    // for some reason a datasize of 30 is encoded as 29
    // ie SPI_DATASIZE_5BIT == 4, etc.
    // we might also consider going the enum route for this one, but it'll be LONG
    uint32_t datasize = config_.datasize - 1;
    if(datasize < 3 || datasize > 31)
    {
        return Result::ERR;
    }

    uint32_t clock_polarity;
    switch(config_.clock_polarity)
    {
        case Config::ClockPolarity::LOW:
            clock_polarity = SPI_POLARITY_LOW;
            break;
        case Config::ClockPolarity::HIGH:
            clock_polarity = SPI_POLARITY_HIGH;
            break;
        default: return Result::ERR;
    }

    uint32_t clock_phase;
    switch(config_.clock_phase)
    {
        case Config::ClockPhase::ONE_EDGE: clock_phase = SPI_PHASE_1EDGE; break;
        case Config::ClockPhase::TWO_EDGE: clock_phase = SPI_PHASE_2EDGE; break;
        default: return Result::ERR;
    }

    uint32_t nss;
    switch(config_.nss)
    {
        case Config::NSS::SOFT: nss = SPI_NSS_SOFT; break;
        case Config::NSS::HARD_INPUT: nss = SPI_NSS_HARD_INPUT; break;
        case Config::NSS::HARD_OUTPUT: nss = SPI_NSS_HARD_OUTPUT; break;
        default: return Result::ERR;
    }

    uint32_t baud_prescaler;
    switch(config_.baud_prescaler)
    {
        case Config::BaudPrescaler::PS_2:
            baud_prescaler = SPI_BAUDRATEPRESCALER_2;
            break;
        case Config::BaudPrescaler::PS_4:
            baud_prescaler = SPI_BAUDRATEPRESCALER_4;
            break;
        case Config::BaudPrescaler::PS_8:
            baud_prescaler = SPI_BAUDRATEPRESCALER_8;
            break;
        case Config::BaudPrescaler::PS_16:
            baud_prescaler = SPI_BAUDRATEPRESCALER_16;
            break;
        case Config::BaudPrescaler::PS_32:
            baud_prescaler = SPI_BAUDRATEPRESCALER_32;
            break;
        case Config::BaudPrescaler::PS_64:
            baud_prescaler = SPI_BAUDRATEPRESCALER_64;
            break;
        case Config::BaudPrescaler::PS_128:
            baud_prescaler = SPI_BAUDRATEPRESCALER_128;
            break;
        case Config::BaudPrescaler::PS_256:
            baud_prescaler = SPI_BAUDRATEPRESCALER_256;
            break;
        default: return Result::ERR;
    }

    hspi_.Instance               = periph;
    hspi_.Init.Mode              = mode;
    hspi_.Init.Direction         = direction;
    hspi_.Init.DataSize          = datasize;
    hspi_.Init.CLKPolarity       = clock_polarity;
    hspi_.Init.CLKPhase          = clock_phase;
    hspi_.Init.NSS               = nss;
    hspi_.Init.BaudRatePrescaler = baud_prescaler;
    hspi_.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    hspi_.Init.TIMode            = SPI_TIMODE_DISABLE;
    hspi_.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    hspi_.Init.CRCPolynomial     = 0x0;
    hspi_.Init.NSSPMode          = SPI_NSS_PULSE_ENABLE;
    hspi_.Init.NSSPolarity       = SPI_NSS_POLARITY_LOW;
    hspi_.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_01DATA;
    hspi_.Init.TxCRCInitializationPattern
        = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi_.Init.RxCRCInitializationPattern
        = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi_.Init.MasterSSIdleness        = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi_.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi_.Init.MasterReceiverAutoSusp  = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi_.Init.MasterKeepIOState       = SPI_MASTER_KEEP_IO_STATE_ENABLE;
    hspi_.Init.IOSwap                  = SPI_IO_SWAP_DISABLE;
    if(HAL_SPI_Init(&hspi_) != HAL_OK)
    {
        Error_Handler();
        return SpiHandle::Result::ERR;
    }

    return SpiHandle::Result::OK;
}

SpiHandle::Result SpiHandle::Impl::SetDmaPeripheral()
{
    switch(config_.periph)
    {
        case SpiHandle::Config::Peripheral::SPI_1:
            hdma_spi_rx_.Init.Request = DMA_REQUEST_SPI1_RX;
            hdma_spi_tx_.Init.Request = DMA_REQUEST_SPI1_TX;
            break;
        case SpiHandle::Config::Peripheral::SPI_2:
            hdma_spi_rx_.Init.Request = DMA_REQUEST_SPI2_RX;
            hdma_spi_tx_.Init.Request = DMA_REQUEST_SPI2_TX;
            break;
        case SpiHandle::Config::Peripheral::SPI_3:
            hdma_spi_rx_.Init.Request = DMA_REQUEST_SPI3_RX;
            hdma_spi_tx_.Init.Request = DMA_REQUEST_SPI3_TX;
            break;
        case SpiHandle::Config::Peripheral::SPI_4:
            hdma_spi_rx_.Init.Request = DMA_REQUEST_SPI4_RX;
            hdma_spi_tx_.Init.Request = DMA_REQUEST_SPI4_TX;
            break;
        case SpiHandle::Config::Peripheral::SPI_5:
            hdma_spi_rx_.Init.Request = DMA_REQUEST_SPI5_RX;
            hdma_spi_tx_.Init.Request = DMA_REQUEST_SPI5_TX;
            break;
        // DMA_REQUEST_SPI6_TX is not available?
        default: return SpiHandle::Result::ERR;
    }
    return SpiHandle::Result::OK;
}

SpiHandle::Result SpiHandle::Impl::InitDma()
{
    hdma_spi_rx_.Instance                 = DMA2_Stream2;
    hdma_spi_rx_.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_spi_rx_.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_spi_rx_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi_rx_.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_spi_rx_.Init.Mode                = DMA_NORMAL;
    hdma_spi_rx_.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    hdma_spi_rx_.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    hdma_spi_rx_.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_spi_rx_.Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma_spi_rx_.Init.PeriphBurst         = DMA_PBURST_SINGLE;

    hdma_spi_tx_.Instance                 = DMA2_Stream3;
    hdma_spi_tx_.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_spi_tx_.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_spi_tx_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi_tx_.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_spi_tx_.Init.Mode                = DMA_NORMAL;
    hdma_spi_tx_.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    hdma_spi_tx_.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    hdma_spi_tx_.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_spi_tx_.Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma_spi_tx_.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    SetDmaPeripheral();

    hdma_spi_rx_.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi_tx_.Init.Direction = DMA_MEMORY_TO_PERIPH;

    if(HAL_DMA_Init(&hdma_spi_rx_) != HAL_OK)
    {
        Error_Handler();
        return SpiHandle::Result::ERR;
    }
    if(HAL_DMA_Init(&hdma_spi_tx_) != HAL_OK)
    {
        Error_Handler();
        return SpiHandle::Result::ERR;
    }

    __HAL_LINKDMA(&hspi_, hdmarx, hdma_spi_rx_);
    __HAL_LINKDMA(&hspi_, hdmatx, hdma_spi_tx_);

    return SpiHandle::Result::OK;
}


void SpiHandle::Impl::DmaTransferFinished(SPI_HandleTypeDef* hspi,
                                          SpiHandle::Result  result)
{
    ScopedIrqBlocker block;

    // on an error, reinit the peripheral to clear any flags
    if(result != SpiHandle::Result::OK)
        HAL_SPI_Init(hspi);

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

    // dma is still idle. Check if another SPI peripheral waits for a job.
    for(int per = 0; per < kNumSpiWithDma; per++)
        if(IsDmaTransferQueuedFor(per))
        {
            SpiHandle::Result result;
            if(queued_dma_transfers_[per].direction
               == SpiHandle::DmaDirection::TX)
            {
                result = spi_handles[per].StartDmaTx(
                    queued_dma_transfers_[per].data_tx,
                    queued_dma_transfers_[per].size,
                    queued_dma_transfers_[per].start_callback,
                    queued_dma_transfers_[per].end_callback,
                    queued_dma_transfers_[per].callback_context);
            }
            else if(queued_dma_transfers_[per].direction
                    == SpiHandle::DmaDirection::RX)
            {
                result = spi_handles[per].StartDmaRx(
                    queued_dma_transfers_[per].data_rx,
                    queued_dma_transfers_[per].size,
                    queued_dma_transfers_[per].start_callback,
                    queued_dma_transfers_[per].end_callback,
                    queued_dma_transfers_[per].callback_context);
            }
            else
            {
                result = spi_handles[per].StartDmaRxTx(
                    queued_dma_transfers_[per].data_rx,
                    queued_dma_transfers_[per].data_tx,
                    queued_dma_transfers_[per].size,
                    queued_dma_transfers_[per].start_callback,
                    queued_dma_transfers_[per].end_callback,
                    queued_dma_transfers_[per].callback_context);
            }
            if(result == SpiHandle::Result::OK)
            {
                // remove the job from the queue
                queued_dma_transfers_[per].Invalidate();
                return;
            }
        }
}


int SpiHandle::Impl::CheckError()
{
    return HAL_SPI_GetError(&hspi_);
}


bool SpiHandle::Impl::IsDmaBusy()
{
    return dma_active_peripheral_ >= 0;
}

bool SpiHandle::Impl::IsDmaTransferQueuedFor(size_t spi_idx)
{
    return queued_dma_transfers_[spi_idx].IsValidJob();
}

void SpiHandle::Impl::QueueDmaTransfer(size_t spi_idx, const SpiDmaJob& job)
{
    // wait for any previous job on this peripheral to finish
    // and the queue position to become free
    while(IsDmaTransferQueuedFor(spi_idx))
    {
        continue;
    };

    // queue the job
    ScopedIrqBlocker block;
    queued_dma_transfers_[spi_idx] = job;
}


SpiHandle::Result
SpiHandle::Impl::DmaTransmit(uint8_t*                            buff,
                             size_t                              size,
                             SpiHandle::StartCallbackFunctionPtr start_callback,
                             SpiHandle::EndCallbackFunctionPtr   end_callback,
                             void* callback_context)
{
    // if dma is currently running - queue a job
    if(IsDmaBusy())
    {
        SpiDmaJob job;
        job.data_tx          = buff;
        job.size             = size;
        job.direction        = SpiHandle::DmaDirection::TX;
        job.start_callback   = start_callback;
        job.end_callback     = end_callback;
        job.callback_context = callback_context;

        const int spi_idx = int(config_.periph);

        // queue a job (blocks until the queue position is free)
        QueueDmaTransfer(spi_idx, job);
        // TODO: the user can't tell if he got returned "OK"
        // because the transfer was executed or because it was queued...
        // should we change that?
        return SpiHandle::Result::OK;
    }

    return StartDmaTx(
        buff, size, start_callback, end_callback, callback_context);
}

SpiHandle::Result
SpiHandle::Impl::StartDmaTx(uint8_t*                            buff,
                            size_t                              size,
                            SpiHandle::StartCallbackFunctionPtr start_callback,
                            SpiHandle::EndCallbackFunctionPtr   end_callback,
                            void* callback_context)
{
    while(HAL_SPI_GetState(&hspi_) != HAL_SPI_STATE_READY) {};

    if(InitDma() != SpiHandle::Result::OK)
    {
        if(end_callback)
            end_callback(callback_context, SpiHandle::Result::ERR);
        return SpiHandle::Result::ERR;
    }

    ScopedIrqBlocker block;

    dma_active_peripheral_ = int(config_.periph);
    next_end_callback_     = end_callback;
    next_callback_context_ = callback_context;

    if(start_callback)
        start_callback(callback_context);

    if(HAL_SPI_Transmit_DMA(&hspi_, buff, size) != HAL_OK)
    {
        dma_active_peripheral_ = -1;
        next_end_callback_     = NULL;
        next_callback_context_ = NULL;
        if(end_callback)
            end_callback(callback_context, SpiHandle::Result::ERR);
        return SpiHandle::Result::ERR;
    }
    return SpiHandle::Result::OK;
}

SpiHandle::Result
SpiHandle::Impl::DmaReceive(uint8_t*                            buff,
                            size_t                              size,
                            SpiHandle::StartCallbackFunctionPtr start_callback,
                            SpiHandle::EndCallbackFunctionPtr   end_callback,
                            void* callback_context)
{
    // if dma is currently running - queue a job
    if(IsDmaBusy())
    {
        SpiDmaJob job;
        job.data_rx          = buff;
        job.size             = size;
        job.direction        = SpiHandle::DmaDirection::RX;
        job.start_callback   = start_callback;
        job.end_callback     = end_callback;
        job.callback_context = callback_context;

        const int spi_idx = int(config_.periph);

        // queue a job (blocks until the queue position is free)
        QueueDmaTransfer(spi_idx, job);
        // TODO: the user can't tell if he got returned "OK"
        // because the transfer was executed or because it was queued...
        // should we change that?
        return SpiHandle::Result::OK;
    }

    return StartDmaRx(
        buff, size, start_callback, end_callback, callback_context);
}

SpiHandle::Result
SpiHandle::Impl::StartDmaRx(uint8_t*                            buff,
                            size_t                              size,
                            SpiHandle::StartCallbackFunctionPtr start_callback,
                            SpiHandle::EndCallbackFunctionPtr   end_callback,
                            void* callback_context)
{
    while(HAL_SPI_GetState(&hspi_) != HAL_SPI_STATE_READY) {};

    if(InitDma() != SpiHandle::Result::OK)
    {
        if(end_callback)
            end_callback(callback_context, SpiHandle::Result::ERR);
        return SpiHandle::Result::ERR;
    }

    ScopedIrqBlocker block;

    dma_active_peripheral_ = int(config_.periph);
    next_end_callback_     = end_callback;
    next_callback_context_ = callback_context;

    if(start_callback)
        start_callback(callback_context);

    if(HAL_SPI_Receive_DMA(&hspi_, buff, size) != HAL_OK)
    {
        dma_active_peripheral_ = -1;
        next_end_callback_     = NULL;
        next_callback_context_ = NULL;
        if(end_callback)
            end_callback(callback_context, SpiHandle::Result::ERR);
        return SpiHandle::Result::ERR;
    }
    return SpiHandle::Result::OK;
}

SpiHandle::Result SpiHandle::Impl::DmaTransmitAndReceive(
    uint8_t*                            tx_buff,
    uint8_t*                            rx_buff,
    size_t                              size,
    SpiHandle::StartCallbackFunctionPtr start_callback,
    SpiHandle::EndCallbackFunctionPtr   end_callback,
    void*                               callback_context)
{
    // if dma is currently running - queue a job
    if(IsDmaBusy())
    {
        SpiDmaJob job;
        job.data_rx          = rx_buff;
        job.data_tx          = tx_buff;
        job.size             = size;
        job.direction        = SpiHandle::DmaDirection::RX_TX;
        job.start_callback   = start_callback;
        job.end_callback     = end_callback;
        job.callback_context = callback_context;

        const int spi_idx = int(config_.periph);

        // queue a job (blocks until the queue position is free)
        QueueDmaTransfer(spi_idx, job);
        // TODO: the user can't tell if he got returned "OK"
        // because the transfer was executed or because it was queued...
        // should we change that?
        return SpiHandle::Result::OK;
    }

    return StartDmaRxTx(
        rx_buff, tx_buff, size, start_callback, end_callback, callback_context);
}

SpiHandle::Result SpiHandle::Impl::StartDmaRxTx(
    uint8_t*                            rx_buff,
    uint8_t*                            tx_buff,
    size_t                              size,
    SpiHandle::StartCallbackFunctionPtr start_callback,
    SpiHandle::EndCallbackFunctionPtr   end_callback,
    void*                               callback_context)
{
    while(HAL_SPI_GetState(&hspi_) != HAL_SPI_STATE_READY) {};

    if(InitDma() != SpiHandle::Result::OK)
    {
        if(end_callback)
            end_callback(callback_context, SpiHandle::Result::ERR);
        return SpiHandle::Result::ERR;
    }

    ScopedIrqBlocker block;

    dma_active_peripheral_ = int(config_.periph);
    next_end_callback_     = end_callback;
    next_callback_context_ = callback_context;

    if(start_callback)
        start_callback(callback_context);

    if(HAL_SPI_TransmitReceive_DMA(&hspi_, tx_buff, rx_buff, size) != HAL_OK)
    {
        dma_active_peripheral_ = -1;
        next_end_callback_     = NULL;
        next_callback_context_ = NULL;
        if(end_callback)
            end_callback(callback_context, SpiHandle::Result::ERR);
        return SpiHandle::Result::ERR;
    }
    return SpiHandle::Result::OK;
}


SpiHandle::Result
SpiHandle::Impl::BlockingTransmit(uint8_t* buff, size_t size, uint32_t timeout)
{
    if(HAL_SPI_Transmit(&hspi_, buff, size, timeout) != HAL_OK)
    {
        return SpiHandle::Result::ERR;
    }
    return SpiHandle::Result::OK;
}

SpiHandle::Result SpiHandle::Impl::BlockingReceive(uint8_t* buffer,
                                                   uint16_t size,
                                                   uint32_t timeout)
{
    if(HAL_SPI_Receive(&hspi_, buffer, size, timeout) != HAL_OK)
    {
        return Result::ERR;
    }
    return Result::OK;
}

SpiHandle::Result SpiHandle::Impl::BlockingTransmitAndReceive(uint8_t* tx_buff,
                                                              uint8_t* rx_buff,
                                                              size_t   size,
                                                              uint32_t timeout)
{
    if(HAL_SPI_TransmitReceive(&hspi_, tx_buff, rx_buff, size, timeout)
       != HAL_OK)
    {
        return SpiHandle::Result::ERR;
    }
    return SpiHandle::Result::OK;
}

typedef struct
{
    dsy_gpio_pin pin;
    uint8_t      alt;
} pin_alt_spi;

static pin_alt_spi pins_none_spi = {{DSY_GPIOX, 0}, 255};

//this is a bit long...
/* ============== spi1 ============== */
static pin_alt_spi spi1_pins_sclk[] = {{{DSY_GPIOG, 11}, GPIO_AF5_SPI1},
                                       {{DSY_GPIOA, 5}, GPIO_AF5_SPI1},
                                       pins_none_spi};

static pin_alt_spi spi1_pins_miso[] = {{{DSY_GPIOB, 4}, GPIO_AF5_SPI1},
                                       {{DSY_GPIOA, 6}, GPIO_AF5_SPI1},
                                       {{DSY_GPIOG, 9}, GPIO_AF5_SPI1}};

static pin_alt_spi spi1_pins_mosi[] = {{{DSY_GPIOB, 5}, GPIO_AF5_SPI1},
                                       {{DSY_GPIOA, 7}, GPIO_AF5_SPI1},
                                       pins_none_spi};

static pin_alt_spi spi1_pins_nss[] = {{{DSY_GPIOG, 10}, GPIO_AF5_SPI1},
                                      {{DSY_GPIOA, 4}, GPIO_AF5_SPI1},
                                      pins_none_spi};

/* ============== spi2 ============== */
static pin_alt_spi spi2_pins_sclk[]
    = {{{DSY_GPIOD, 3}, GPIO_AF5_SPI2}, pins_none_spi, pins_none_spi};

static pin_alt_spi spi2_pins_miso[] = {{{DSY_GPIOB, 14}, GPIO_AF5_SPI2},
                                       {{DSY_GPIOC, 2}, GPIO_AF5_SPI2},
                                       pins_none_spi};

static pin_alt_spi spi2_pins_mosi[] = {{{DSY_GPIOC, 1}, GPIO_AF5_SPI2},
                                       {{DSY_GPIOB, 15}, GPIO_AF5_SPI2},
                                       {{DSY_GPIOC, 3}, GPIO_AF5_SPI2}};

static pin_alt_spi spi2_pins_nss[] = {{{DSY_GPIOB, 12}, GPIO_AF5_SPI2},
                                      {{DSY_GPIOB, 4}, GPIO_AF7_SPI2},
                                      {{DSY_GPIOB, 9}, GPIO_AF5_SPI2}};

/* ============== spi3 ============== */
static pin_alt_spi spi3_pins_sclk[]
    = {{{DSY_GPIOC, 10}, GPIO_AF6_SPI3}, pins_none_spi, pins_none_spi};

static pin_alt_spi spi3_pins_miso[] = {{{DSY_GPIOC, 11}, GPIO_AF6_SPI3},
                                       {{DSY_GPIOB, 4}, GPIO_AF6_SPI3},
                                       pins_none_spi};

static pin_alt_spi spi3_pins_mosi[] = {{{DSY_GPIOC, 12}, GPIO_AF6_SPI3},
                                       {{DSY_GPIOB, 5}, GPIO_AF7_SPI3},
                                       pins_none_spi};

static pin_alt_spi spi3_pins_nss[]
    = {{{DSY_GPIOA, 4}, GPIO_AF6_SPI3}, pins_none_spi, pins_none_spi};

/* ============== spi4 ============== */
static pin_alt_spi spi4_pins_sclk[]
    = {pins_none_spi, pins_none_spi, pins_none_spi};

static pin_alt_spi spi4_pins_miso[]
    = {pins_none_spi, pins_none_spi, pins_none_spi};

static pin_alt_spi spi4_pins_mosi[]
    = {pins_none_spi, pins_none_spi, pins_none_spi};

static pin_alt_spi spi4_pins_nss[]
    = {pins_none_spi, pins_none_spi, pins_none_spi};

/* ============== spi5 ============== */
static pin_alt_spi spi5_pins_sclk[]
    = {pins_none_spi, pins_none_spi, pins_none_spi};

static pin_alt_spi spi5_pins_miso[]
    = {pins_none_spi, pins_none_spi, pins_none_spi};

static pin_alt_spi spi5_pins_mosi[]
    = {pins_none_spi, pins_none_spi, pins_none_spi};

static pin_alt_spi spi5_pins_nss[]
    = {pins_none_spi, pins_none_spi, pins_none_spi};

/* ============== spi6 ============== */
static pin_alt_spi spi6_pins_sclk[]
    = {{{DSY_GPIOA, 5}, GPIO_AF8_SPI6}, pins_none_spi, pins_none_spi};

static pin_alt_spi spi6_pins_miso[] = {{{DSY_GPIOB, 4}, GPIO_AF8_SPI6},
                                       {{DSY_GPIOA, 6}, GPIO_AF8_SPI6},
                                       pins_none_spi};

static pin_alt_spi spi6_pins_mosi[] = {{{DSY_GPIOB, 5}, GPIO_AF8_SPI6},
                                       {{DSY_GPIOA, 7}, GPIO_AF8_SPI6},
                                       pins_none_spi};

static pin_alt_spi spi6_pins_nss[]
    = {{{DSY_GPIOA, 4}, GPIO_AF8_SPI6}, pins_none_spi, pins_none_spi};

//an array to hold everything
static pin_alt_spi* pins_periphs_spi[] = {
    spi1_pins_sclk, spi1_pins_miso, spi1_pins_mosi, spi1_pins_nss,
    spi2_pins_sclk, spi2_pins_miso, spi2_pins_mosi, spi2_pins_nss,
    spi3_pins_sclk, spi3_pins_miso, spi3_pins_mosi, spi3_pins_nss,
    spi4_pins_sclk, spi4_pins_miso, spi4_pins_mosi, spi4_pins_nss,
    spi5_pins_sclk, spi5_pins_miso, spi5_pins_mosi, spi5_pins_nss,
    spi6_pins_sclk, spi6_pins_miso, spi6_pins_mosi, spi6_pins_nss,
};

SpiHandle::Result
checkPinMatchSpi(GPIO_InitTypeDef* init, dsy_gpio_pin pin, int p_num)
{
    for(int i = 0; i < 3; i++)
    {
        if(dsy_pin_cmp(&pins_periphs_spi[p_num][i].pin, &pins_none_spi.pin))
        {
            /* skip */
        }

        else if(dsy_pin_cmp(&pins_periphs_spi[p_num][i].pin, &pin))
        {
            init->Alternate = pins_periphs_spi[p_num][i].alt;
            return SpiHandle::Result::OK;
        }
    }

    return SpiHandle::Result::ERR;
}

// here's how the directions work:

// full duplex == two_line
// (master) mosi -> mosi (slave)
// (master) miso <- miso (slave)

// half duplex == one_line
// (master) mosi <--> miso (slave)

// simplex tx == two_line_tx
// (master) mosi ->     (or)
// (slave)  miso ->

// simplex rx == two_line_rx
// (master) miso <-     (or)
// (slave)  mosi <-

SpiHandle::Result SpiHandle::Impl::InitPins()
{
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    int per_num = 4 * (int)(config_.periph);

    bool is_master = config_.mode == Config::Mode::MASTER;

    //  slave all except 2linerx, master 2line and 2linerx
    bool enable_miso
        = (!is_master
           && config_.direction != Config::Direction::TWO_LINES_RX_ONLY)
          || (is_master
              && config_.direction == Config::Direction::TWO_LINES_RX_ONLY)
          || (is_master && config_.direction == Config::Direction::TWO_LINES);

    //  master all except 2linerx, slave 2linerx and 2line
    bool enable_mosi
        = (is_master
           && config_.direction != Config::Direction::TWO_LINES_RX_ONLY)
          || (!is_master
              && config_.direction == Config::Direction::TWO_LINES_RX_ONLY)
          || (!is_master && config_.direction == Config::Direction::TWO_LINES);

    // nss = soft -> ss pin is unused for master and slave
    bool enable_ss = config_.nss != Config::NSS::SOFT;

    if(config_.pin_config.sclk.port != DSY_GPIOX)
    {
        //check sclk against periph
        if(checkPinMatchSpi(&GPIO_InitStruct, config_.pin_config.sclk, per_num)
           == Result::ERR)
        {
            return Result::ERR;
        }

        //setup sclk pin
        GPIO_TypeDef* port  = dsy_hal_map_get_port(&config_.pin_config.sclk);
        GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&config_.pin_config.sclk);
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    }

    if(config_.pin_config.miso.port != DSY_GPIOX && enable_miso)
    {
        //check miso against periph
        if(checkPinMatchSpi(
               &GPIO_InitStruct, config_.pin_config.miso, per_num + 1)
           == Result::ERR)
        {
            return Result::ERR;
        }

        //setup miso pin
        GPIO_TypeDef* port  = dsy_hal_map_get_port(&config_.pin_config.miso);
        GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&config_.pin_config.miso);
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    }

    if(config_.pin_config.mosi.port != DSY_GPIOX && enable_mosi)
    {
        //check mosi against periph
        if(checkPinMatchSpi(
               &GPIO_InitStruct, config_.pin_config.mosi, per_num + 2)
           == Result::ERR)
        {
            return Result::ERR;
        }

        //setup mosi pin
        GPIO_TypeDef* port  = dsy_hal_map_get_port(&config_.pin_config.mosi);
        GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&config_.pin_config.mosi);
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    }

    if(config_.pin_config.nss.port != DSY_GPIOX && enable_ss)
    {
        //check nss against periph
        if(checkPinMatchSpi(
               &GPIO_InitStruct, config_.pin_config.nss, per_num + 3)
           == Result::ERR)
        {
            return Result::ERR;
        }

        //setup nss pin
        GPIO_TypeDef* port  = dsy_hal_map_get_port(&config_.pin_config.nss);
        GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&config_.pin_config.nss);
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    }

    return Result::OK;
}

SpiHandle::Result SpiHandle::Impl::DeInitPins()
{
    GPIO_TypeDef* port = dsy_hal_map_get_port(&config_.pin_config.sclk);
    uint16_t      pin  = dsy_hal_map_get_pin(&config_.pin_config.sclk);
    HAL_GPIO_DeInit(port, pin);

    port = dsy_hal_map_get_port(&config_.pin_config.miso);
    pin  = dsy_hal_map_get_pin(&config_.pin_config.miso);
    HAL_GPIO_DeInit(port, pin);

    port = dsy_hal_map_get_port(&config_.pin_config.mosi);
    pin  = dsy_hal_map_get_pin(&config_.pin_config.mosi);
    HAL_GPIO_DeInit(port, pin);

    port = dsy_hal_map_get_port(&config_.pin_config.nss);
    pin  = dsy_hal_map_get_pin(&config_.pin_config.nss);
    HAL_GPIO_DeInit(port, pin);

    return Result::OK;
}

volatile int8_t SpiHandle::Impl::dma_active_peripheral_;
SpiHandle::Impl::SpiDmaJob
    SpiHandle::Impl::queued_dma_transfers_[kNumSpiWithDma];

SpiHandle::EndCallbackFunctionPtr SpiHandle::Impl::next_end_callback_;
void*                             SpiHandle::Impl::next_callback_context_;

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{
    SpiHandle::Impl* handle = MapInstanceToHandle(spiHandle->Instance);

    //enable clock on all 4 pins
    dsy_hal_map_gpio_clk_enable(handle->config_.pin_config.sclk.port);
    dsy_hal_map_gpio_clk_enable(handle->config_.pin_config.miso.port);
    dsy_hal_map_gpio_clk_enable(handle->config_.pin_config.mosi.port);
    dsy_hal_map_gpio_clk_enable(handle->config_.pin_config.nss.port);

    //enable clock for our peripheral
    switch(handle->config_.periph)
    {
        case SpiHandle::Config::Peripheral::SPI_1:
        {
            __HAL_RCC_SPI1_CLK_ENABLE();
            HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(SPI1_IRQn);
        }
        break;
        case SpiHandle::Config::Peripheral::SPI_2:
        {
            __HAL_RCC_SPI2_CLK_ENABLE();
            HAL_NVIC_SetPriority(SPI2_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(SPI2_IRQn);
        }
        break;
        case SpiHandle::Config::Peripheral::SPI_3:
        {
            __HAL_RCC_SPI3_CLK_ENABLE();
            HAL_NVIC_SetPriority(SPI3_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(SPI3_IRQn);
        }
        break;
        case SpiHandle::Config::Peripheral::SPI_4:
        {
            __HAL_RCC_SPI4_CLK_ENABLE();
            HAL_NVIC_SetPriority(SPI4_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(SPI4_IRQn);
        }
        break;
        case SpiHandle::Config::Peripheral::SPI_5:
        {
            __HAL_RCC_SPI5_CLK_ENABLE();
            HAL_NVIC_SetPriority(SPI5_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(SPI5_IRQn);
        }
        break;
        case SpiHandle::Config::Peripheral::SPI_6:
            __HAL_RCC_SPI6_CLK_ENABLE();
            break;
    }


    if(handle->InitPins() == SpiHandle::Result::ERR)
    {
        Error_Handler();
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{
    SpiHandle::Impl* handle = MapInstanceToHandle(spiHandle->Instance);

    //disable clock for our peripheral
    switch(handle->config_.periph)
    {
        case SpiHandle::Config::Peripheral::SPI_1:
            __HAL_RCC_SPI1_CLK_DISABLE();
            break;
        case SpiHandle::Config::Peripheral::SPI_2:
            __HAL_RCC_SPI2_CLK_DISABLE();
            break;
        case SpiHandle::Config::Peripheral::SPI_3:
            __HAL_RCC_SPI3_CLK_DISABLE();
            break;
        case SpiHandle::Config::Peripheral::SPI_4:
            __HAL_RCC_SPI4_CLK_DISABLE();
            break;
        case SpiHandle::Config::Peripheral::SPI_5:
            __HAL_RCC_SPI5_CLK_DISABLE();
            break;
        case SpiHandle::Config::Peripheral::SPI_6:
            __HAL_RCC_SPI6_CLK_DISABLE();
            break;
    }

    if(handle->DeInitPins() == SpiHandle::Result::ERR)
    {
        Error_Handler();
    }
}

extern "C" void dsy_spi_global_init()
{
    SpiHandle::Impl::GlobalInit();
}

extern "C" void SPI1_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&spi_handles[0].hspi_);
}

extern "C" void SPI2_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&spi_handles[1].hspi_);
}

extern "C" void SPI3_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&spi_handles[2].hspi_);
}

extern "C" void SPI4_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&spi_handles[3].hspi_);
}

extern "C" void SPI5_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&spi_handles[4].hspi_);
}

void HalSpiDmaRxStreamCallback(void)
{
    ScopedIrqBlocker block;
    if(SpiHandle::Impl::dma_active_peripheral_ >= 0)
        HAL_DMA_IRQHandler(
            &spi_handles[SpiHandle::Impl::dma_active_peripheral_].hdma_spi_rx_);
}
extern "C" void DMA2_Stream2_IRQHandler(void)
{
    HalSpiDmaRxStreamCallback();
}
void HalSpiDmaTxStreamCallback(void)
{
    ScopedIrqBlocker block;
    if(SpiHandle::Impl::dma_active_peripheral_ >= 0)
        HAL_DMA_IRQHandler(
            &spi_handles[SpiHandle::Impl::dma_active_peripheral_].hdma_spi_tx_);
}
extern "C" void DMA2_Stream3_IRQHandler(void)
{
    HalSpiDmaTxStreamCallback();
}

extern "C" void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi)
{
    SpiHandle::Impl::DmaTransferFinished(hspi, SpiHandle::Result::OK);
}

extern "C" void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef* hspi)
{
    SpiHandle::Impl::DmaTransferFinished(hspi, SpiHandle::Result::OK);
}

extern "C" void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef* hspi)
{
    SpiHandle::Impl::DmaTransferFinished(hspi, SpiHandle::Result::OK);
}

extern "C" void HAL_SPI_ErrorCallback(SPI_HandleTypeDef* hspi)
{
    SpiHandle::Impl::DmaTransferFinished(hspi, SpiHandle::Result::ERR);
}

// ======================================================================
// SpiHandler > SpiHandlePimpl
// ======================================================================

SpiHandle::Result SpiHandle::Init(const Config& config)
{
    pimpl_ = &spi_handles[int(config.periph)];
    return pimpl_->Init(config);
}

const SpiHandle::Config& SpiHandle::GetConfig() const
{
    return pimpl_->GetConfig();
}

int SpiHandle::CheckError()
{
    return pimpl_->CheckError();
}


SpiHandle::Result
SpiHandle::BlockingTransmit(uint8_t* buff, size_t size, uint32_t timeout)
{
    return pimpl_->BlockingTransmit(buff, size, timeout);
}

SpiHandle::Result
SpiHandle::BlockingReceive(uint8_t* buffer, uint16_t size, uint32_t timeout)
{
    return pimpl_->BlockingReceive(buffer, size, timeout);
}


SpiHandle::Result
SpiHandle::DmaTransmit(uint8_t*                            buff,
                       size_t                              size,
                       SpiHandle::StartCallbackFunctionPtr start_callback,
                       SpiHandle::EndCallbackFunctionPtr   end_callback,
                       void*                               callback_context)
{
    return pimpl_->DmaTransmit(
        buff, size, start_callback, end_callback, callback_context);
}

SpiHandle::Result
SpiHandle::DmaReceive(uint8_t*                            buff,
                      size_t                              size,
                      SpiHandle::StartCallbackFunctionPtr start_callback,
                      SpiHandle::EndCallbackFunctionPtr   end_callback,
                      void*                               callback_context)
{
    return pimpl_->DmaReceive(
        buff, size, start_callback, end_callback, callback_context);
}
SpiHandle::Result SpiHandle::DmaTransmitAndReceive(
    uint8_t*                            rx_buff,
    uint8_t*                            tx_buff,
    size_t                              size,
    SpiHandle::StartCallbackFunctionPtr start_callback,
    SpiHandle::EndCallbackFunctionPtr   end_callback,
    void*                               callback_context)
{
    return pimpl_->DmaTransmitAndReceive(
        rx_buff, tx_buff, size, start_callback, end_callback, callback_context);
}

SpiHandle::Result SpiHandle::BlockingTransmitAndReceive(uint8_t* tx_buff,
                                                        uint8_t* rx_buff,
                                                        size_t   size,
                                                        uint32_t timeout)
{
    return pimpl_->BlockingTransmitAndReceive(tx_buff, rx_buff, size, timeout);
}
