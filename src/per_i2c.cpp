#include "per_i2c.h"
extern "C"
{
#include "util_hal_map.h"
}

namespace daisy
{
/** Private implementation for I2CHandle */
class I2CHandle::Impl
{
  public:
    I2CHandle::Result        init(const I2CHandle::Config& config);
    const I2CHandle::Config& getConfig() const { return config_; }

    I2CHandle::Result transmitBlocking(uint16_t address,
                                       uint8_t* data,
                                       uint16_t size,
                                       uint32_t timeout);
    I2CHandle::Result transmitDma(uint16_t                       address,
                                  uint8_t*                       data,
                                  uint16_t                       size,
                                  I2CHandle::CallbackFunctionPtr callback,
                                  void* callback_context);

    // =========================================================
    // scheduling and global functions
    struct DmaJob
    {
        uint16_t                       slave_address    = 0;
        uint8_t*                       data             = nullptr;
        uint16_t                       size             = 0;
        I2CHandle::CallbackFunctionPtr callback         = nullptr;
        void*                          callback_context = nullptr;

        bool isValidJob() const { return data != nullptr; }
    };
    static void globalInit();
    static bool isDmaActive();
    static bool isDmaTransferQueuedFor(size_t i2c_peripheral_idx);
    static void queueDmaTransfer(size_t i2c_peripheral_idx, const DmaJob& job);
    static void dmaTransferFinished(void*             hal_i2c_handle,
                                    I2CHandle::Result result);

    static constexpr uint8_t              kNumI2CWithDma = 3;
    static volatile int8_t                dma_active_peripheral_;
    static DmaJob                         queued_dma_transfers_[kNumI2CWithDma];
    static I2CHandle::CallbackFunctionPtr next_callback_;
    static void*                          next_callback_context_;

    // =========================================================
    // pivate functions and member variables
    I2CHandle::Config config_;
    DMA_HandleTypeDef i2c_dma_tx_handle_;
    I2C_HandleTypeDef i2c_hal_handle_;

    I2CHandle::Result startDmaTransfer(uint16_t                       address,
                                       uint8_t*                       data,
                                       uint16_t                       size,
                                       I2CHandle::CallbackFunctionPtr callback,
                                       void* callback_context);

    void initPins();
    void deinitPins();
};


// ======================================================================
// Error handler
// ======================================================================

static void Error_Handler()
{
    asm("bkpt 255");
    while(1) {}
}

// ================================================================
// Global references for the availabel I2CHandle::Impls
// ================================================================

static I2CHandle::Impl i2c_handles[4];

// ================================================================
// Scheduling and global functions
// ================================================================

void I2CHandle::Impl::globalInit()
{
    // init the scheduler queue
    dma_active_peripheral_ = -1;
    for(int per = 0; per < kNumI2CWithDma; per++)
        queued_dma_transfers_[per] = I2CHandle::Impl::DmaJob();
}

bool I2CHandle::Impl::isDmaActive()
{
    return dma_active_peripheral_ >= 0;
}

bool I2CHandle::Impl::isDmaTransferQueuedFor(size_t i2c_peripheral_idx)
{
    return queued_dma_transfers_[i2c_peripheral_idx].isValidJob();
}

void I2CHandle::Impl::queueDmaTransfer(size_t i2c_peripheral_idx,
                                       const I2CHandle::Impl::DmaJob& job)
{
    // wait for any previous job on this peripheral to finish
    // and the queue position to bevome free
    while(isDmaTransferQueuedFor(i2c_peripheral_idx)) {};

    // queue the job
    // TODO: Add ScopedIrqBlocker here
    queued_dma_transfers_[i2c_peripheral_idx] = job;
}

void I2CHandle::Impl::dmaTransferFinished(void*             hal_i2c_handle,
                                          I2CHandle::Result result)
{
    // TODO: Add ScopedIrqBlocker

    I2C_HandleTypeDef* hal_hi2c = (I2C_HandleTypeDef*)hal_i2c_handle;

    // on an error, reinit the peripheral to clear any flags
    if(result != I2CHandle::Result::OK)
        HAL_I2C_Init(hal_hi2c);

    dma_active_peripheral_ = -1;

    if(next_callback_ != nullptr)
    {
        // the callback may setup another transmission, hence we shouldn't reset this to
        // nullptr after the callback - it might overwrite the new transmission.
        auto callback  = next_callback_;
        next_callback_ = nullptr;
        // make the callback
        callback(next_callback_context_, result);
    }

    // the callback could have started a new transmission right away...
    if(isDmaActive())
        return;

    // dma is still idle. Check if another i2c peripheral waits for a job.
    for(int per = 0; per < kNumI2CWithDma; per++)
        if(isDmaTransferQueuedFor(per))
        {
            if(i2c_handles[per].startDmaTransfer(
                   queued_dma_transfers_[per].slave_address,
                   queued_dma_transfers_[per].data,
                   queued_dma_transfers_[per].size,
                   queued_dma_transfers_[per].callback,
                   queued_dma_transfers_[per].callback_context)
               == I2CHandle::Result::OK)
            {
                // remove the job from the queue
                queued_dma_transfers_[per].data = nullptr;
                return;
            }
        }
}

// ================================================================
// I2C functions
// ================================================================

I2CHandle::Result I2CHandle::Impl::init(const I2CHandle::Config& config)
{
    const int i2cIdx = int(config.periph);

    if(i2cIdx >= 4)
        return I2CHandle::Result::ERR;

    config_ = config;
    constexpr I2C_TypeDef* instances[4]
        = {I2C1, I2C2, I2C3, I2C4}; // map HAL instances
    i2c_hal_handle_.Instance = instances[i2cIdx];

    // Set Generic Parameters
    // Configure Speed
    // TODO: make this dependent on the current I2C clock speed set in sys
    switch(config_.speed)
    {
        case I2CHandle::Config::Speed::I2C_100KHZ:
            i2c_hal_handle_.Init.Timing = 0x30E0628A;
            break;
        case I2CHandle::Config::Speed::I2C_400KHZ:
            i2c_hal_handle_.Init.Timing = 0x20D01132;
            break;
        case I2CHandle::Config::Speed::I2C_1MHZ:
            i2c_hal_handle_.Init.Timing = 0x1080091A;
            break;
        default: break;
    }
    //	i2c_hal_handle_.Init.Timing = 0x00C0EAFF;
    i2c_hal_handle_.Init.OwnAddress1      = 0;
    i2c_hal_handle_.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    i2c_hal_handle_.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    i2c_hal_handle_.Init.OwnAddress2      = 0;
    i2c_hal_handle_.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    i2c_hal_handle_.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    i2c_hal_handle_.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
    if(HAL_I2C_Init(&i2c_hal_handle_) != HAL_OK)
        return I2CHandle::Result::ERR;
    if(HAL_I2CEx_ConfigAnalogFilter(&i2c_hal_handle_, I2C_ANALOGFILTER_ENABLE)
       != HAL_OK)
        return I2CHandle::Result::ERR;
    if(HAL_I2CEx_ConfigDigitalFilter(&i2c_hal_handle_, 0) != HAL_OK)
        return I2CHandle::Result::ERR;

    return I2CHandle::Result::OK;
}

I2CHandle::Result I2CHandle::Impl::transmitBlocking(uint16_t address,
                                                    uint8_t* data,
                                                    uint16_t size,
                                                    uint32_t timeout)
{
    // wait for previous transfer to be finished
    while(HAL_I2C_GetState(&i2c_hal_handle_) != HAL_I2C_STATE_READY) {};

    if(HAL_I2C_Master_Transmit(
           &i2c_hal_handle_, address << 1, data, size, timeout)
       != HAL_OK)
        return I2CHandle::Result::ERR;
    return I2CHandle::Result::OK;
}

I2CHandle::Result
I2CHandle::Impl::transmitDma(uint16_t                       address,
                             uint8_t*                       data,
                             uint16_t                       size,
                             I2CHandle::CallbackFunctionPtr callback,
                             void*                          callback_context)
{
    // I2C4 has no DMA yet.
    if(config_.periph == I2CHandle::Config::Peripheral::I2C_4)
        return I2CHandle::Result::ERR;

    const int i2cIdx = int(config_.periph);

    // if dma is currently running - queue a job
    if(isDmaActive())
    {
        DmaJob job;
        job.slave_address    = address;
        job.data             = data;
        job.size             = size;
        job.callback         = callback;
        job.callback_context = callback_context;
        // queue a job (blocks until the queue position is free)
        queueDmaTransfer(i2cIdx, job);
        // TODO: the user can't tell if he got returned "OK"
        // because the transfer was executed or because it was queued...
        // should we change that?
        return I2CHandle::Result::OK;
    }
    else
        // start transmission right away
        return startDmaTransfer(
            address, data, size, callback, callback_context);
}

I2CHandle::Result
I2CHandle::Impl::startDmaTransfer(uint16_t                       address,
                                  uint8_t*                       data,
                                  uint16_t                       size,
                                  I2CHandle::CallbackFunctionPtr callback,
                                  void* callback_context)
{
    // this could be called from both the scheduler ISR
    // and from user code via dsy_i2c_transmit_dma()
    // TODO: Add some sort of locking mechanism.

    // wait for previous transfer to be finished
    while(HAL_I2C_GetState(&i2c_hal_handle_) != HAL_I2C_STATE_READY) {};

    // reinit the DMA
    i2c_dma_tx_handle_.Instance = DMA1_Stream6;
    switch(config_.periph)
    {
        case I2CHandle::Config::Peripheral::I2C_1:
            i2c_dma_tx_handle_.Init.Request = DMA_REQUEST_I2C1_TX;
            break;
        case I2CHandle::Config::Peripheral::I2C_2:
            i2c_dma_tx_handle_.Init.Request = DMA_REQUEST_I2C2_TX;
            break;
        case I2CHandle::Config::Peripheral::I2C_3:
            i2c_dma_tx_handle_.Init.Request = DMA_REQUEST_I2C3_TX;
            break;
        // I2C4 has no DMA yet. TODO
        default: return I2CHandle::Result::ERR;
    }
    i2c_dma_tx_handle_.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    i2c_dma_tx_handle_.Init.PeriphInc           = DMA_PINC_DISABLE;
    i2c_dma_tx_handle_.Init.MemInc              = DMA_MINC_ENABLE;
    i2c_dma_tx_handle_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    i2c_dma_tx_handle_.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    i2c_dma_tx_handle_.Init.Mode                = DMA_NORMAL;
    i2c_dma_tx_handle_.Init.Priority            = DMA_PRIORITY_LOW;
    i2c_dma_tx_handle_.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    i2c_dma_tx_handle_.Init.MemBurst            = DMA_MBURST_SINGLE;
    i2c_dma_tx_handle_.Init.PeriphBurst         = DMA_PBURST_SINGLE;

    if(HAL_DMA_Init(&i2c_dma_tx_handle_) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(&i2c_hal_handle_, hdmatx, i2c_dma_tx_handle_);

    // start the transfer
    // TODO: Add ScopedIrqBlocker
    dma_active_peripheral_ = int(config_.periph);
    next_callback_         = callback;
    next_callback_context_ = callback_context;
    if(HAL_I2C_Master_Transmit_DMA(&i2c_hal_handle_, address << 1, data, size)
       != HAL_OK)
    {
        dma_active_peripheral_ = -1;
        next_callback_         = NULL;
        next_callback_context_ = NULL;
        return I2CHandle::Result::ERR;
    }
    return I2CHandle::Result::OK;
}

void I2CHandle::Impl::initPins()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_TypeDef*    port;

    GPIO_InitStruct.Mode  = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    switch(config_.periph)
    {
        case I2CHandle::Config::Peripheral::I2C_1:
            GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
            break;
        case I2CHandle::Config::Peripheral::I2C_2:
            GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
            break;
        case I2CHandle::Config::Peripheral::I2C_3:
            GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
            break;
        case I2CHandle::Config::Peripheral::I2C_4:
            GPIO_InitStruct.Alternate = GPIO_AF4_I2C4;
            break;
        default: break;
    }

    port                = dsy_hal_map_get_port(&config_.pin_config.scl);
    GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&config_.pin_config.scl);
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    port                = dsy_hal_map_get_port(&config_.pin_config.sda);
    GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&config_.pin_config.sda);
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void I2CHandle::Impl::deinitPins()
{
    GPIO_TypeDef* port;
    uint16_t      pin;
    port = dsy_hal_map_get_port(&config_.pin_config.scl);
    pin  = dsy_hal_map_get_pin(&config_.pin_config.scl);
    HAL_GPIO_DeInit(port, pin);
    port = dsy_hal_map_get_port(&config_.pin_config.sda);
    pin  = dsy_hal_map_get_pin(&config_.pin_config.sda);
    HAL_GPIO_DeInit(port, pin);
}

volatile int8_t         I2CHandle::Impl::dma_active_peripheral_;
I2CHandle::Impl::DmaJob I2CHandle::Impl::queued_dma_transfers_[kNumI2CWithDma];
I2CHandle::CallbackFunctionPtr I2CHandle::Impl::next_callback_;
void*                          I2CHandle::Impl::next_callback_context_;

// ======================================================================
// HAL service functions
// ======================================================================

extern "C" void HAL_I2C_MspInit(I2C_HandleTypeDef* i2c_handle)
{
    if(i2c_handle->Instance == I2C1)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        i2c_handles[0].initPins();
        __HAL_RCC_I2C1_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    }
    else if(i2c_handle->Instance == I2C2)
    {
        __HAL_RCC_GPIOH_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        i2c_handles[1].initPins();
        __HAL_RCC_I2C2_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        HAL_NVIC_SetPriority(I2C2_EV_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
    }
    else if(i2c_handle->Instance == I2C3)
    {
        // Enable RCC GPIO CLK for necessary ports.
        i2c_handles[2].initPins();
        __HAL_RCC_I2C3_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        HAL_NVIC_SetPriority(I2C3_EV_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);
    }
    else if(i2c_handle->Instance == I2C4)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        i2c_handles[3].initPins();
        __HAL_RCC_I2C4_CLK_ENABLE();

        // I2C4 needs BMDA
        // TODO
    }
}

extern "C" void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2c_handle)
{
    if(i2c_handle->Instance == I2C1)
    {
        __HAL_RCC_I2C1_CLK_DISABLE();
        i2c_handles[0].deinitPins();
    }
    else if(i2c_handle->Instance == I2C2)
    {
        __HAL_RCC_I2C2_CLK_DISABLE();
        i2c_handles[1].deinitPins();
    }
    else if(i2c_handle->Instance == I2C3)
    {
        // Enable RCC GPIO CLK for necessary ports.
        __HAL_RCC_I2C3_CLK_DISABLE();
        i2c_handles[2].deinitPins();
    }
    else if(i2c_handle->Instance == I2C4)
    {
        __HAL_RCC_I2C4_CLK_DISABLE();
        i2c_handles[3].deinitPins();
    }
}

extern "C" void dsy_i2c_global_init()
{
    I2CHandle::Impl::globalInit();
}

// ======================================================================
// ISRs and event handlers
// ======================================================================

// we need this intermediary function to be able to call into
// private functions of the I2CHandle objects...
void halI2CDmaStreamCallback(void)
{
    // TODO: add ScopedIrqBlocker
    if(I2CHandle::Impl::dma_active_peripheral_ >= 0)
        HAL_DMA_IRQHandler(&i2c_handles[I2CHandle::Impl::dma_active_peripheral_]
                                .i2c_dma_tx_handle_);
}
extern "C" void DMA1_Stream6_IRQHandler(void)
{
    halI2CDmaStreamCallback();
}

extern "C" void I2C1_EV_IRQHandler()
{
    HAL_I2C_EV_IRQHandler(&i2c_handles[0].i2c_hal_handle_);
}

extern "C" void I2C2_EV_IRQHandler()
{
    HAL_I2C_EV_IRQHandler(&i2c_handles[1].i2c_hal_handle_);
}

extern "C" void I2C3_EV_IRQHandler()
{
    HAL_I2C_EV_IRQHandler(&i2c_handles[2].i2c_hal_handle_);
}

extern "C" void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef* i2c_handle)
{
    I2CHandle::Impl::dmaTransferFinished(i2c_handle, I2CHandle::Result::OK);
}

extern "C" void HAL_I2C_ErrorCallback(I2C_HandleTypeDef* i2c_handle)
{
    I2CHandle::Impl::dmaTransferFinished(i2c_handle, I2CHandle::Result::ERR);
}

// ======================================================================
// I2CHandle > I2CHandlePimpl
// ======================================================================

I2CHandle::Result I2CHandle::init(const I2CHandle::Config& config)
{
    // set the pimpl pointer
    pimpl_ = &i2c_handles[int(config.periph)];
    return pimpl_->init(config);
}

const I2CHandle::Config& I2CHandle::getConfig() const
{
    return pimpl_->getConfig();
}

I2CHandle::Result I2CHandle::transmitBlocking(uint16_t address,
                                              uint8_t* data,
                                              uint16_t size,
                                              uint32_t timeout)
{
    return pimpl_->transmitBlocking(address, data, size, timeout);
}

I2CHandle::Result
I2CHandle::transmitDma(uint16_t                       address,
                       uint8_t*                       data,
                       uint16_t                       size,
                       I2CHandle::CallbackFunctionPtr callback,
                       void*                          callback_context)
{
    return pimpl_->transmitDma(address, data, size, callback, callback_context);
}

} // namespace daisy

#if 0
#include "util_hal_map.h"

// ======================================================================
// global variables
// ======================================================================

// TODO: This is global, and the board gets set for each init.
// Its a bit redundant, but I'm just trying to validate some hardware
//   without breaking all the other boards.
static dsy_i2c_handle i2c_handles[4];
// I2C4 needs BDMA. TODO: Add this.
static DMA_HandleTypeDef i2c_dma_tx_handles[3];

// ======================================================================
// Error handler
// ======================================================================

static void Error_Handler()
{
    asm("bkpt 255");
    while(1) {}
}

// ======================================================================
// private types
// ======================================================================

typedef enum
{
    dsy_i2c_transfer_success,
    dsy_i2c_transfer_failure
} dsy_i2c_transfer_result;

// ======================================================================
// dma job scheduling (all i2c peripherals share a single DMA stream)
// ======================================================================

/** Scheduling for DMA the transfers, as all I2C peripherals share the same DMA. 
 */
typedef struct
{
    uint16_t                       slave_address;
    uint8_t*                       data;
    uint16_t                       size;
    dsy_i2c_transf_cplt_callback_t callback;
    void*                          callback_context;
} DmaTransferJob;

// I2C4 has no DMA assigned yet.
// TODO: change this to 4 once that's done.
static const uint8_t num_i2c_with_dma = 3;

/** < 0 if DMA is not yet active
  * 0..3 if DMA is currently busy with the corresponding i2c peripheral
  */
static volatile int8_t dma_active_peripheral;
/** dma transfers that are waiting to be executed; one for each i2c peripheral. */
static volatile DmaTransferJob queued_dma_transfers[4];
/** callback for the current transfer */
static dsy_i2c_transf_cplt_callback_t current_callback;
static void*                          current_callback_context;

/** Returns true, if the i2c dma is currently executing a job. */
static int is_dma_active()
{
    return dma_active_peripheral >= 0;
}

/** Returns true, if a transfer job is queued for the provided i2c peripheral. */
static int is_dma_transfer_queued(size_t i2c_peripheral_idx)
{
    return queued_dma_transfers[i2c_peripheral_idx].data != NULL;
}

/** Queues a DmaTransferJob to be executed later. 
 * TODO: If the same peripheral is used from multiple threads
 * of execution (main loop or ISRs) we should add some sort of syncronisation
 * to this function.
 */
static void queue_dma_transfer(size_t                i2c_peripheral_idx,
                               const DmaTransferJob* job)
{
    // wait for any previous job on this peripheral to finish
    // and the queue position to bevome free
    while(is_dma_transfer_queued(i2c_peripheral_idx)) {};

    // queue the job
    queued_dma_transfers[i2c_peripheral_idx] = *job;
}

/** starts a dma transfer */
static dsy_i2c_result
start_dma_transfer(dsy_i2c_handle*                dsy_hi2c,
                   uint16_t                       address,
                   uint8_t*                       data,
                   uint16_t                       size,
                   dsy_i2c_transf_cplt_callback_t callback,
                   void*                          callback_context)
{
    // this could be called from both the scheduler ISR
    // and from user code via dsy_i2c_transmit_dma()
    // TODO: Add some sort of locking mechanism.

    const int          i2cIdx   = dsy_hi2c->config.periph;
    I2C_HandleTypeDef* hal_hi2c = (I2C_HandleTypeDef*)dsy_hi2c->hal_hi2c;
    // wait for previous transfer to be finished
    while(HAL_I2C_GetState(hal_hi2c) != HAL_I2C_STATE_READY) {};

    // reinit the DMA
    i2c_dma_tx_handles[i2cIdx].Instance = DMA1_Stream6;
    switch(i2cIdx)
    {
        case DSY_I2C_PERIPH_1:
            i2c_dma_tx_handles[i2cIdx].Init.Request = DMA_REQUEST_I2C1_TX;
            break;
        case DSY_I2C_PERIPH_2:
            i2c_dma_tx_handles[i2cIdx].Init.Request = DMA_REQUEST_I2C2_TX;
            break;
        case DSY_I2C_PERIPH_3:
            i2c_dma_tx_handles[i2cIdx].Init.Request = DMA_REQUEST_I2C3_TX;
            break;
        // I2C4 has no DMA yet. TODO
        default: return DSY_I2C_RES_ERR;
    }
    i2c_dma_tx_handles[i2cIdx].Init.Direction           = DMA_MEMORY_TO_PERIPH;
    i2c_dma_tx_handles[i2cIdx].Init.PeriphInc           = DMA_PINC_DISABLE;
    i2c_dma_tx_handles[i2cIdx].Init.MemInc              = DMA_MINC_ENABLE;
    i2c_dma_tx_handles[i2cIdx].Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    i2c_dma_tx_handles[i2cIdx].Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    i2c_dma_tx_handles[i2cIdx].Init.Mode                = DMA_NORMAL;
    i2c_dma_tx_handles[i2cIdx].Init.Priority            = DMA_PRIORITY_LOW;
    i2c_dma_tx_handles[i2cIdx].Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    i2c_dma_tx_handles[i2cIdx].Init.MemBurst            = DMA_MBURST_SINGLE;
    i2c_dma_tx_handles[i2cIdx].Init.PeriphBurst         = DMA_PBURST_SINGLE;

    if(HAL_DMA_Init(&i2c_dma_tx_handles[i2cIdx]) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(hal_hi2c, hdmatx, i2c_dma_tx_handles[i2cIdx]);


    // start the transfer
    dma_active_peripheral    = i2cIdx;
    current_callback         = callback;
    current_callback_context = callback_context;
    if(HAL_I2C_Master_Transmit_DMA(hal_hi2c, address << 1, data, size)
       != HAL_OK)
    {
        dma_active_peripheral    = -1;
        current_callback         = NULL;
        current_callback_context = NULL;
        return DSY_I2C_RES_ERR;
    }
    return DSY_I2C_RES_OK;
}

/** dma scheduler */
static void dma_transfer_finished(I2C_HandleTypeDef*      i2c_handle,
                                  dsy_i2c_transfer_result result)
{
    dma_active_peripheral = -1;

    // on an error, reinit the peripheral to clear any flags
    if(result == dsy_i2c_transfer_failure)
        HAL_I2C_Init(i2c_handle);

    if(current_callback != NULL)
    {
        // the callback may setup another transmission, hence we shouldn't reset this to NULL
        // after the callback - it might overwrite the new transmission.
        dsy_i2c_transf_cplt_callback_t callback = current_callback;
        current_callback                        = NULL;
        // make the callback
        callback(current_callback_context,
                 (result == dsy_i2c_transfer_success) ? DSY_I2C_RES_OK
                                                      : DSY_I2C_RES_ERR);
    }

    // the callback could have started a new transmission right away...
    if(is_dma_active())
        return;

    // dma is still idle. Check if another i2c peripheral waits for a job.
    for(int per = 0; per < num_i2c_with_dma; per++)
        if(is_dma_transfer_queued(per))
        {
            if(start_dma_transfer(&i2c_handles[per],
                                  queued_dma_transfers[per].slave_address,
                                  queued_dma_transfers[per].data,
                                  queued_dma_transfers[per].size,
                                  queued_dma_transfers[per].callback,
                                  queued_dma_transfers[per].callback_context)
               == DSY_I2C_RES_OK)
            {
                // remove the job from the queue
                queued_dma_transfers[per].data = NULL;
                return;
            }
        }
}

// ======================================================================
// i2c driver functions
// ======================================================================

void dsy_i2c_global_init()
{
    dma_active_peripheral = -1;
    // init the scheduler queue
    for(int per = 0; per < num_i2c_with_dma; per++)
        queued_dma_transfers[per].data = NULL;
}

dsy_i2c_result dsy_i2c_init(dsy_i2c_handle* dsy_hi2c)
{
    I2C_HandleTypeDef* hal_hi2c = dsy_hal_map_get_i2c(&dsy_hi2c->config);
    dsy_hi2c->hal_hi2c          = (void*)hal_hi2c;
    switch(dsy_hi2c->config.periph)
    {
        default:
        case DSY_I2C_PERIPH_1:
            i2c_handles[0]     = *dsy_hi2c;
            hal_hi2c->Instance = I2C1;
            break;
        case DSY_I2C_PERIPH_2:
            i2c_handles[1]     = *dsy_hi2c;
            hal_hi2c->Instance = I2C2;
            break;
        case DSY_I2C_PERIPH_3:
            i2c_handles[2]     = *dsy_hi2c;
            hal_hi2c->Instance = I2C3;
            break;
        case DSY_I2C_PERIPH_4:
            i2c_handles[3]     = *dsy_hi2c;
            hal_hi2c->Instance = I2C4;
            break;
    }
    // Set Generic Parameters
    // Configure Speed
    // TODO: make this dependent on the current I2C Clock speed set in sys
    switch(dsy_hi2c->config.speed)
    {
        case DSY_I2C_SPEED_100KHZ: hal_hi2c->Init.Timing = 0x30E0628A; break;
        case DSY_I2C_SPEED_400KHZ: hal_hi2c->Init.Timing = 0x20D01132; break;
        case DSY_I2C_SPEED_1MHZ: hal_hi2c->Init.Timing = 0x1080091A; break;
        default: break;
    }
    //	hal_hi2c->Init.Timing = 0x00C0EAFF;
    hal_hi2c->Init.OwnAddress1      = 0;
    hal_hi2c->Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    hal_hi2c->Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    hal_hi2c->Init.OwnAddress2      = 0;
    hal_hi2c->Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hal_hi2c->Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    hal_hi2c->Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
    if(HAL_I2C_Init(hal_hi2c) != HAL_OK)
        return DSY_I2C_RES_ERR;
    if(HAL_I2CEx_ConfigAnalogFilter(hal_hi2c, I2C_ANALOGFILTER_ENABLE)
       != HAL_OK)
        return DSY_I2C_RES_ERR;
    if(HAL_I2CEx_ConfigDigitalFilter(hal_hi2c, 0) != HAL_OK)
        return DSY_I2C_RES_ERR;

    return DSY_I2C_RES_OK;
}

static void init_i2c_pins(dsy_i2c_handle* hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_TypeDef*    port;

    GPIO_InitStruct.Mode  = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    switch(hi2c->config.periph)
    {
        case DSY_I2C_PERIPH_1: GPIO_InitStruct.Alternate = GPIO_AF4_I2C1; break;
        case DSY_I2C_PERIPH_2: GPIO_InitStruct.Alternate = GPIO_AF4_I2C2; break;
        case DSY_I2C_PERIPH_3: GPIO_InitStruct.Alternate = GPIO_AF4_I2C3; break;
        case DSY_I2C_PERIPH_4: GPIO_InitStruct.Alternate = GPIO_AF4_I2C4; break;
        default: break;
    }

    port                = dsy_hal_map_get_port(&hi2c->config.pin_config.scl);
    GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&hi2c->config.pin_config.scl);
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    port                = dsy_hal_map_get_port(&hi2c->config.pin_config.sda);
    GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&hi2c->config.pin_config.sda);
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

static void deinit_i2c_pins(dsy_i2c_handle* hi2c)
{
    GPIO_TypeDef* port;
    uint16_t      pin;
    port = dsy_hal_map_get_port(&hi2c->config.pin_config.scl);
    pin  = dsy_hal_map_get_pin(&hi2c->config.pin_config.scl);
    HAL_GPIO_DeInit(port, pin);
    port = dsy_hal_map_get_port(&hi2c->config.pin_config.sda);
    pin  = dsy_hal_map_get_pin(&hi2c->config.pin_config.sda);
    HAL_GPIO_DeInit(port, pin);
}

dsy_i2c_result dsy_i2c_transmit_blocking(dsy_i2c_handle* dsy_hi2c,
                                         uint16_t        address,
                                         uint8_t*        pData,
                                         uint16_t        size,
                                         uint32_t        timeout)
{
    I2C_HandleTypeDef* hal_hi2c = (I2C_HandleTypeDef*)dsy_hi2c->hal_hi2c;

    // wait for previous transfer to be finished
    while(HAL_I2C_GetState(hal_hi2c) != HAL_I2C_STATE_READY) {};

    if(HAL_I2C_Master_Transmit(hal_hi2c, address << 1, pData, size, timeout)
       != HAL_OK)
        return DSY_I2C_RES_ERR;
    return DSY_I2C_RES_OK;
}

dsy_i2c_result dsy_i2c_transmit_dma(dsy_i2c_handle*                dsy_hi2c,
                                    uint16_t                       address,
                                    uint8_t*                       data,
                                    uint16_t                       size,
                                    dsy_i2c_transf_cplt_callback_t callback,
                                    void* callback_context)
{
    // I2C4 has no DMA yet.
    if(dsy_hi2c->config.periph == DSY_I2C_PERIPH_4)
        return DSY_I2C_RES_ERR;

    // if dma is currently running - queue a job
    if(is_dma_active())
    {
        DmaTransferJob job;
        job.slave_address    = address;
        job.data             = data;
        job.size             = size;
        job.callback         = callback;
        job.callback_context = callback_context;
        // queue a job (blocks until the queue position is free)
        queue_dma_transfer(dsy_hi2c->config.periph, &job);
        // TODO: the user can't tell if he got returned "OK"
        // because the transfer was executed or because it was queued...
        // should we change that?
        return DSY_I2C_RES_OK;
    }
    else
        // start transmission right away
        return start_dma_transfer(
            dsy_hi2c, address, data, size, callback, callback_context);
}

dsy_i2c_result dsy_i2c_ready(dsy_i2c_handle* dsy_hi2c)
{
    I2C_HandleTypeDef* hal_hi2c = (I2C_HandleTypeDef*)dsy_hi2c->hal_hi2c;
    if(HAL_I2C_GetState(hal_hi2c) != HAL_I2C_STATE_READY)
        return DSY_I2C_RES_ERR;
    return DSY_I2C_RES_OK;
}

// ======================================================================
// HAL service functions
// ======================================================================

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2c_handle)
{
    if(i2c_handle->Instance == I2C1)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        init_i2c_pins(&i2c_handles[0]);
        __HAL_RCC_I2C1_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    }
    else if(i2c_handle->Instance == I2C2)
    {
        __HAL_RCC_GPIOH_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        init_i2c_pins(&i2c_handles[1]);
        __HAL_RCC_I2C2_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        HAL_NVIC_SetPriority(I2C2_EV_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
    }
    else if(i2c_handle->Instance == I2C3)
    {
        // Enable RCC GPIO CLK for necessary ports.
        init_i2c_pins(&i2c_handles[2]);
        __HAL_RCC_I2C3_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        HAL_NVIC_SetPriority(I2C3_EV_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);
    }
    else if(i2c_handle->Instance == I2C4)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        init_i2c_pins(&i2c_handles[3]);
        __HAL_RCC_I2C4_CLK_ENABLE();

        // I2C4 needs BMDA
        // TODO
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2c_handle)
{
    if(i2c_handle->Instance == I2C1)
    {
        __HAL_RCC_I2C1_CLK_DISABLE();
        deinit_i2c_pins(&i2c_handles[0]);
    }
    else if(i2c_handle->Instance == I2C2)
    {
        __HAL_RCC_I2C2_CLK_DISABLE();
        deinit_i2c_pins(&i2c_handles[1]);
    }
    else if(i2c_handle->Instance == I2C3)
    {
        // Enable RCC GPIO CLK for necessary ports.
        __HAL_RCC_I2C3_CLK_DISABLE();
        deinit_i2c_pins(&i2c_handles[2]);
    }
    else if(i2c_handle->Instance == I2C4)
    {
        __HAL_RCC_I2C4_CLK_DISABLE();
        deinit_i2c_pins(&i2c_handles[3]);
    }
}

// ======================================================================
// ISRs and event handlers
// ======================================================================

void DMA1_Stream6_IRQHandler(void)
{
    // TODO: multiplex this to handle all I2C peripherals
    if(dma_active_peripheral >= 0)
        HAL_DMA_IRQHandler(&i2c_dma_tx_handles[dma_active_peripheral]);
}

void I2C1_EV_IRQHandler()
{
    HAL_I2C_EV_IRQHandler(i2c_handles[0].hal_hi2c);
}

void I2C2_EV_IRQHandler()
{
    HAL_I2C_EV_IRQHandler(i2c_handles[1].hal_hi2c);
}

void I2C3_EV_IRQHandler()
{
    HAL_I2C_EV_IRQHandler(i2c_handles[2].hal_hi2c);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef* i2c_handle)
{
    dma_transfer_finished(i2c_handle, dsy_i2c_transfer_success);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef* i2c_handle)
{
    dma_transfer_finished(i2c_handle, dsy_i2c_transfer_failure);
}
#endif