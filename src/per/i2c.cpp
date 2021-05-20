#include "per/i2c.h"
#include "sys/system.h"
#include "util/scopedirqblocker.h"
extern "C"
{
#include "util/hal_map.h"
}

namespace daisy
{
/** Private implementation for I2CHandle */
class I2CHandle::Impl
{
  public:
    I2CHandle::Result        Init(const I2CHandle::Config& config);
    const I2CHandle::Config& GetConfig() const { return config_; }

    I2CHandle::Result TransmitBlocking(uint16_t address,
                                       uint8_t* data,
                                       uint16_t size,
                                       uint32_t timeout);

    I2CHandle::Result TransmitDma(uint16_t                       address,
                                  uint8_t*                       data,
                                  uint16_t                       size,
                                  I2CHandle::CallbackFunctionPtr callback,
                                  void* callback_context);

    I2CHandle::Result ReceiveBlocking(uint16_t address,
                                      uint8_t* data,
                                      uint16_t size,
                                      uint32_t timeout);

    I2CHandle::Result ReceiveDma(uint16_t                       address,
                                 uint8_t*                       data,
                                 uint16_t                       size,
                                 I2CHandle::CallbackFunctionPtr callback,
                                 void* callback_context);

    I2CHandle::Result ReadDataAtAddress(uint16_t address,
                                        uint16_t mem_address,
                                        uint16_t mem_address_size,
                                        uint8_t* data,
                                        uint16_t data_size,
                                        uint32_t timeout);

    I2CHandle::Result WriteDataAtAddress(uint16_t address,
                                         uint16_t mem_address,
                                         uint16_t mem_address_size,
                                         uint8_t* data,
                                         uint16_t data_size,
                                         uint32_t timeout);

    // =========================================================
    // scheduling and global functions
    struct DmaJob
    {
        uint16_t                       slave_address    = 0x10;
        uint8_t*                       data             = nullptr;
        uint16_t                       size             = 0;
        I2CHandle::CallbackFunctionPtr callback         = nullptr;
        void*                          callback_context = nullptr;
        I2CHandle::Direction direction = I2CHandle::Direction::TRANSMIT;

        bool IsValidJob() const { return data != nullptr; }
        void Invalidate() { data = nullptr; }
    };
    static void GlobalInit();
    static bool IsDmaActive();
    static bool IsDmaTransferQueuedFor(size_t i2c_peripheral_idx);
    static void QueueDmaTransfer(size_t i2c_peripheral_idx, const DmaJob& job);
    static void DmaTransferFinished(I2C_HandleTypeDef* hal_i2c_handle,
                                    I2CHandle::Result  result);

    static constexpr uint8_t              kNumI2CWithDma = 3;
    static volatile int8_t                dma_active_peripheral_;
    static DmaJob                         queued_dma_transfers_[kNumI2CWithDma];
    static I2CHandle::CallbackFunctionPtr next_callback_;
    static void*                          next_callback_context_;

    // =========================================================
    // pivate functions and member variables
    I2CHandle::Config config_;
    DMA_HandleTypeDef i2c_dma_tc_handle_;
    I2C_HandleTypeDef i2c_hal_handle_;

    I2CHandle::Result
    StartDmaTransmission(uint16_t                       address,
                         uint8_t*                       data,
                         uint16_t                       size,
                         I2CHandle::CallbackFunctionPtr callback,
                         void*                          callback_context);

    I2CHandle::Result StartDmaReception(uint16_t                       address,
                                        uint8_t*                       data,
                                        uint16_t                       size,
                                        I2CHandle::CallbackFunctionPtr callback,
                                        void* callback_context);

    void InitPins();
    void DeinitPins();
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

void I2CHandle::Impl::GlobalInit()
{
    // init the scheduler queue
    dma_active_peripheral_ = -1;
    for(int per = 0; per < kNumI2CWithDma; per++)
        queued_dma_transfers_[per] = I2CHandle::Impl::DmaJob();
}

bool I2CHandle::Impl::IsDmaActive()
{
    return dma_active_peripheral_ >= 0;
}

bool I2CHandle::Impl::IsDmaTransferQueuedFor(size_t i2c_peripheral_idx)
{
    return queued_dma_transfers_[i2c_peripheral_idx].IsValidJob();
}

void I2CHandle::Impl::QueueDmaTransfer(size_t i2c_peripheral_idx,
                                       const I2CHandle::Impl::DmaJob& job)
{
    // wait for any previous job on this peripheral to finish
    // and the queue position to bevome free
    while(IsDmaTransferQueuedFor(i2c_peripheral_idx)) {};

    // queue the job
    ScopedIrqBlocker block;
    queued_dma_transfers_[i2c_peripheral_idx] = job;
}

void I2CHandle::Impl::DmaTransferFinished(I2C_HandleTypeDef* hal_i2c_handle,
                                          I2CHandle::Result  result)
{
    ScopedIrqBlocker block;

    // on an error, reinit the peripheral to clear any flags
    if(result != I2CHandle::Result::OK)
        HAL_I2C_Init(hal_i2c_handle);

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
    if(IsDmaActive())
        return;

    // dma is still idle. Check if another i2c peripheral waits for a job.
    for(int per = 0; per < kNumI2CWithDma; per++)
        if(IsDmaTransferQueuedFor(per))
        {
            I2CHandle::Result result;
            if(queued_dma_transfers_[per].direction
               == I2CHandle::Direction::TRANSMIT)
            {
                result = i2c_handles[per].StartDmaTransmission(
                    queued_dma_transfers_[per].slave_address,
                    queued_dma_transfers_[per].data,
                    queued_dma_transfers_[per].size,
                    queued_dma_transfers_[per].callback,
                    queued_dma_transfers_[per].callback_context);
            }
            else
            {
                result = i2c_handles[per].StartDmaReception(
                    queued_dma_transfers_[per].slave_address,
                    queued_dma_transfers_[per].data,
                    queued_dma_transfers_[per].size,
                    queued_dma_transfers_[per].callback,
                    queued_dma_transfers_[per].callback_context);
            }
            if(result == I2CHandle::Result::OK)
            {
                // remove the job from the queue
                queued_dma_transfers_[per].Invalidate();
                return;
            }
        }
}

// ================================================================
// I2C functions
// ================================================================

I2CHandle::Result I2CHandle::Impl::Init(const I2CHandle::Config& config)
{
    const int i2cIdx = int(config.periph);

    if(i2cIdx >= 4)
        return I2CHandle::Result::ERR;
    if(config.mode != I2CHandle::Config::Mode::I2C_MASTER
       && config.mode != I2CHandle::Config::Mode::I2C_SLAVE)
        return I2CHandle::Result::ERR;
    // Ensuring address is valid and not using reserved addresses
    if(config.mode == I2CHandle::Config::Mode::I2C_SLAVE
       && (config.address > 127 || config.address < 16 || config.address > 119))
        return I2CHandle::Result::ERR;

    config_ = config;
    constexpr I2C_TypeDef* instances[4]
        = {I2C1, I2C2, I2C3, I2C4}; // map HAL instances
    i2c_hal_handle_.Instance = instances[i2cIdx];

    // Set Generic Parameters
    // Configure Speed
    // TODO: Make this check the actual I2C clock source instead of it
    //      "knowing" that it's the PCLK1
    switch(config_.speed)
    {
        case I2CHandle::Config::Speed::I2C_100KHZ:
            if(System::GetPClk1Freq() == 120000000)
                // Measured at 100kHz w/ 4k7 pullup
                i2c_hal_handle_.Init.Timing = 0x6090435F;
            else
                // Measured at 100kHz w/ 4k7 pullup
                i2c_hal_handle_.Init.Timing = 0x30E0628A;
            break;
        case I2CHandle::Config::Speed::I2C_400KHZ:
            if(System::GetPClk1Freq() == 120000000)
                // Measured at 400kHz w/ 4k7 pullup
                i2c_hal_handle_.Init.Timing = 0x30B00F2D;
            else
                // Measured at 400kHz w/ 4k7 pullup
                i2c_hal_handle_.Init.Timing = 0x20D01132;
            break;
        case I2CHandle::Config::Speed::I2C_1MHZ:
            if(System::GetPClk1Freq() == 120000000)
                // Measured at 837kHz w/ 4k7 pullup
                i2c_hal_handle_.Init.Timing = 0x10A00B20;
            else
                // Measured at 862kHz w/ 4k7 pullup
                i2c_hal_handle_.Init.Timing = 0x1080091A;
            break;
        default: break;
    }

    // HAL expects the address to be represented in its I2C
    // format, i.e. shifted to the left by one, since bit 0
    // indicated whether the transmission is a read or write
    i2c_hal_handle_.Init.OwnAddress1      = config.address << 1;
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

I2CHandle::Result I2CHandle::Impl::TransmitBlocking(uint16_t address,
                                                    uint8_t* data,
                                                    uint16_t size,
                                                    uint32_t timeout)
{
    // wait for previous transfer to be finished
    while(HAL_I2C_GetState(&i2c_hal_handle_) != HAL_I2C_STATE_READY) {};

    HAL_StatusTypeDef status;
    if(config_.mode == I2CHandle::Config::Mode::I2C_MASTER)
    {
        status = HAL_I2C_Master_Transmit(
            &i2c_hal_handle_, address << 1, data, size, timeout);
    }
    else
    {
        status = HAL_I2C_Slave_Transmit(&i2c_hal_handle_, data, size, timeout);
    }
    if(status != HAL_OK)
        return I2CHandle::Result::ERR;

    return I2CHandle::Result::OK;
}

I2CHandle::Result
I2CHandle::Impl::TransmitDma(uint16_t                       address,
                             uint8_t*                       data,
                             uint16_t                       size,
                             I2CHandle::CallbackFunctionPtr callback,
                             void*                          callback_context)
{
    // I2C4 has no DMA yet.
    if(config_.periph == I2CHandle::Config::Peripheral::I2C_4)
        return I2CHandle::Result::ERR;

    const int i2cIdx = int(config_.periph);

    I2CHandle::Direction direction = I2CHandle::Direction::TRANSMIT;

    // if dma is currently running - queue a job
    if(IsDmaActive())
    {
        DmaJob job;
        job.slave_address    = address;
        job.data             = data;
        job.size             = size;
        job.direction        = direction;
        job.callback         = callback;
        job.callback_context = callback_context;
        // queue a job (blocks until the queue position is free)
        QueueDmaTransfer(i2cIdx, job);
        // TODO: the user can't tell if he got returned "OK"
        // because the transfer was executed or because it was queued...
        // should we change that?
        return I2CHandle::Result::OK;
    }
    else
        // start transmission right away
        return StartDmaTransmission(
            address, data, size, callback, callback_context);
}

I2CHandle::Result I2CHandle::Impl::ReceiveBlocking(uint16_t address,
                                                   uint8_t* data,
                                                   uint16_t size,
                                                   uint32_t timeout)
{
    // wait for previous transfer to be finished
    while(HAL_I2C_GetState(&i2c_hal_handle_) != HAL_I2C_STATE_READY) {};

    HAL_StatusTypeDef status;
    if(config_.mode == I2CHandle::Config::Mode::I2C_MASTER)
    {
        status = HAL_I2C_Master_Receive(
            &i2c_hal_handle_, address << 1, data, size, timeout);
    }
    else
        status = HAL_I2C_Slave_Receive(&i2c_hal_handle_, data, size, timeout);

    if(status != HAL_OK)
        return I2CHandle::Result::ERR;

    return I2CHandle::Result::OK;
}

I2CHandle::Result
I2CHandle::Impl::ReceiveDma(uint16_t                       address,
                            uint8_t*                       data,
                            uint16_t                       size,
                            I2CHandle::CallbackFunctionPtr callback,
                            void*                          callback_context)
{
    // I2C4 has no DMA yet.
    if(config_.periph == I2CHandle::Config::Peripheral::I2C_4)
        return I2CHandle::Result::ERR;

    const int i2cIdx = int(config_.periph);

    I2CHandle::Direction direction = I2CHandle::Direction::RECEIVE;

    // if dma is currently running - queue a job
    if(IsDmaActive())
    {
        DmaJob job;
        job.slave_address    = address;
        job.data             = data;
        job.size             = size;
        job.direction        = direction;
        job.callback         = callback;
        job.callback_context = callback_context;
        // queue a job (blocks until the queue position is free)
        QueueDmaTransfer(i2cIdx, job);
        // TODO: the user can't tell if he got returned "OK"
        // because the transfer was executed or because it was queued...
        // should we change that?
        return I2CHandle::Result::OK;
    }
    else
        // start reception right away
        return StartDmaReception(
            address, data, size, callback, callback_context);
}

I2CHandle::Result I2CHandle::Impl::ReadDataAtAddress(uint16_t address,
                                                     uint16_t mem_address,
                                                     uint16_t mem_address_size,
                                                     uint8_t* data,
                                                     uint16_t data_size,
                                                     uint32_t timeout)
{
    // Only master devices can make requests
    if(config_.mode != I2CHandle::Config::Mode::I2C_MASTER)
        return I2CHandle::Result::ERR;

    // wait for previous transfer to be finished
    while(HAL_I2C_GetState(&i2c_hal_handle_) != HAL_I2C_STATE_READY) {};
    if(HAL_I2C_Mem_Read(&i2c_hal_handle_,
                        address,
                        mem_address,
                        mem_address_size,
                        data,
                        data_size,
                        timeout)
       != HAL_OK)
    {
        return I2CHandle::Result::ERR;
    }
    return I2CHandle::Result::OK;
}

I2CHandle::Result I2CHandle::Impl::WriteDataAtAddress(uint16_t address,
                                                      uint16_t mem_address,
                                                      uint16_t mem_address_size,
                                                      uint8_t* data,
                                                      uint16_t data_size,
                                                      uint32_t timeout)
{
    // Only master devices can make requests
    if(config_.mode != I2CHandle::Config::Mode::I2C_MASTER)
        return I2CHandle::Result::ERR;

    // wait for previous transfer to be finished
    while(HAL_I2C_GetState(&i2c_hal_handle_) != HAL_I2C_STATE_READY) {};
    if(HAL_I2C_Mem_Write(&i2c_hal_handle_,
                         address,
                         mem_address,
                         mem_address_size,
                         data,
                         data_size,
                         timeout)
       != HAL_OK)
    {
        return I2CHandle::Result::ERR;
    }
    return I2CHandle::Result::OK;
}

I2CHandle::Result
I2CHandle::Impl::StartDmaTransmission(uint16_t                       address,
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
    i2c_dma_tc_handle_.Instance = DMA1_Stream6;
    switch(config_.periph)
    {
        case I2CHandle::Config::Peripheral::I2C_1:
            i2c_dma_tc_handle_.Init.Request = DMA_REQUEST_I2C1_TX;
            break;
        case I2CHandle::Config::Peripheral::I2C_2:
            i2c_dma_tc_handle_.Init.Request = DMA_REQUEST_I2C2_TX;
            break;
        case I2CHandle::Config::Peripheral::I2C_3:
            i2c_dma_tc_handle_.Init.Request = DMA_REQUEST_I2C3_TX;
            break;
        // I2C4 has no DMA yet. TODO
        default: return I2CHandle::Result::ERR;
    }
    i2c_dma_tc_handle_.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    i2c_dma_tc_handle_.Init.PeriphInc           = DMA_PINC_DISABLE;
    i2c_dma_tc_handle_.Init.MemInc              = DMA_MINC_ENABLE;
    i2c_dma_tc_handle_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    i2c_dma_tc_handle_.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    i2c_dma_tc_handle_.Init.Mode                = DMA_NORMAL;
    i2c_dma_tc_handle_.Init.Priority            = DMA_PRIORITY_LOW;
    i2c_dma_tc_handle_.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    i2c_dma_tc_handle_.Init.MemBurst            = DMA_MBURST_SINGLE;
    i2c_dma_tc_handle_.Init.PeriphBurst         = DMA_PBURST_SINGLE;

    if(HAL_DMA_Init(&i2c_dma_tc_handle_) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(&i2c_hal_handle_, hdmatx, i2c_dma_tc_handle_);

    // start the transfer and block irq until return
    ScopedIrqBlocker block;

    dma_active_peripheral_ = int(config_.periph);
    next_callback_         = callback;
    next_callback_context_ = callback_context;

    HAL_StatusTypeDef status;
    if(config_.mode == I2CHandle::Config::Mode::I2C_MASTER)
    {
        status = HAL_I2C_Master_Transmit_DMA(
            &i2c_hal_handle_, address << 1, data, size);
    }
    else
    {
        status = HAL_I2C_Slave_Transmit_DMA(&i2c_hal_handle_, data, size);
    }

    if(status != HAL_OK)
    {
        dma_active_peripheral_ = -1;
        next_callback_         = NULL;
        next_callback_context_ = NULL;
        return I2CHandle::Result::ERR;
    }
    return I2CHandle::Result::OK;
}

I2CHandle::Result
I2CHandle::Impl::StartDmaReception(uint16_t                       address,
                                   uint8_t*                       data,
                                   uint16_t                       size,
                                   I2CHandle::CallbackFunctionPtr callback,
                                   void* callback_context)
{
    // wait for previous transfer to be finished
    while(HAL_I2C_GetState(&i2c_hal_handle_) != HAL_I2C_STATE_READY) {};

    // reinit the DMA
    i2c_dma_tc_handle_.Instance = DMA1_Stream6;
    switch(config_.periph)
    {
        case I2CHandle::Config::Peripheral::I2C_1:
            i2c_dma_tc_handle_.Init.Request = DMA_REQUEST_I2C1_RX;
            break;
        case I2CHandle::Config::Peripheral::I2C_2:
            i2c_dma_tc_handle_.Init.Request = DMA_REQUEST_I2C2_RX;
            break;
        case I2CHandle::Config::Peripheral::I2C_3:
            i2c_dma_tc_handle_.Init.Request = DMA_REQUEST_I2C3_RX;
            break;
        // I2C4 has no DMA yet. TODO
        default: return I2CHandle::Result::ERR;
    }
    i2c_dma_tc_handle_.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    i2c_dma_tc_handle_.Init.PeriphInc           = DMA_PINC_DISABLE;
    i2c_dma_tc_handle_.Init.MemInc              = DMA_MINC_ENABLE;
    i2c_dma_tc_handle_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    i2c_dma_tc_handle_.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    i2c_dma_tc_handle_.Init.Mode                = DMA_NORMAL;
    i2c_dma_tc_handle_.Init.Priority            = DMA_PRIORITY_LOW;
    i2c_dma_tc_handle_.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    i2c_dma_tc_handle_.Init.MemBurst            = DMA_MBURST_SINGLE;
    i2c_dma_tc_handle_.Init.PeriphBurst         = DMA_PBURST_SINGLE;

    if(HAL_DMA_Init(&i2c_dma_tc_handle_) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(&i2c_hal_handle_, hdmarx, i2c_dma_tc_handle_);

    // start the transfer and block irq until return
    ScopedIrqBlocker block;

    dma_active_peripheral_ = int(config_.periph);
    next_callback_         = callback;
    next_callback_context_ = callback_context;

    HAL_StatusTypeDef status;
    if(config_.mode == I2CHandle::Config::Mode::I2C_MASTER)
    {
        status = HAL_I2C_Master_Receive_DMA(
            &i2c_hal_handle_, address << 1, data, size);
    }
    else
    {
        status = HAL_I2C_Slave_Receive_DMA(&i2c_hal_handle_, data, size);
    }

    if(status != HAL_OK)
    {
        dma_active_peripheral_ = -1;
        next_callback_         = NULL;
        next_callback_context_ = NULL;
        return I2CHandle::Result::ERR;
    }
    return I2CHandle::Result::OK;
}

void I2CHandle::Impl::InitPins()
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

void I2CHandle::Impl::DeinitPins()
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
        i2c_handles[0].InitPins();
        __HAL_RCC_I2C1_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    }
    else if(i2c_handle->Instance == I2C2)
    {
        __HAL_RCC_GPIOH_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        i2c_handles[1].InitPins();
        __HAL_RCC_I2C2_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        HAL_NVIC_SetPriority(I2C2_EV_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
    }
    else if(i2c_handle->Instance == I2C3)
    {
        // Enable RCC GPIO CLK for necessary ports.
        i2c_handles[2].InitPins();
        __HAL_RCC_I2C3_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        HAL_NVIC_SetPriority(I2C3_EV_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);
    }
    else if(i2c_handle->Instance == I2C4)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        i2c_handles[3].InitPins();
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
        i2c_handles[0].DeinitPins();
    }
    else if(i2c_handle->Instance == I2C2)
    {
        __HAL_RCC_I2C2_CLK_DISABLE();
        i2c_handles[1].DeinitPins();
    }
    else if(i2c_handle->Instance == I2C3)
    {
        // Enable RCC GPIO CLK for necessary ports.
        __HAL_RCC_I2C3_CLK_DISABLE();
        i2c_handles[2].DeinitPins();
    }
    else if(i2c_handle->Instance == I2C4)
    {
        __HAL_RCC_I2C4_CLK_DISABLE();
        i2c_handles[3].DeinitPins();
    }
}

extern "C" void dsy_i2c_global_init()
{
    I2CHandle::Impl::GlobalInit();
}

// ======================================================================
// ISRs and event handlers
// ======================================================================

// we need this intermediary function to be able to call into
// private functions of the I2CHandle objects...
void halI2CDmaStreamCallback(void)
{
    ScopedIrqBlocker block;
    if(I2CHandle::Impl::dma_active_peripheral_ >= 0)
        HAL_DMA_IRQHandler(&i2c_handles[I2CHandle::Impl::dma_active_peripheral_]
                                .i2c_dma_tc_handle_);
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
    I2CHandle::Impl::DmaTransferFinished(i2c_handle, I2CHandle::Result::OK);
}

extern "C" void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* i2c_handle)
{
    I2CHandle::Impl::DmaTransferFinished(i2c_handle, I2CHandle::Result::OK);
}

extern "C" void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef* i2c_handle)
{
    I2CHandle::Impl::DmaTransferFinished(i2c_handle, I2CHandle::Result::OK);
}

extern "C" void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef* i2c_handle)
{
    I2CHandle::Impl::DmaTransferFinished(i2c_handle, I2CHandle::Result::OK);
}

extern "C" void HAL_I2C_ErrorCallback(I2C_HandleTypeDef* i2c_handle)
{
    I2CHandle::Impl::DmaTransferFinished(i2c_handle, I2CHandle::Result::ERR);
}

// ======================================================================
// I2CHandle > I2CHandlePimpl
// ======================================================================

I2CHandle::Result I2CHandle::Init(const I2CHandle::Config& config)
{
    // set the pimpl pointer
    pimpl_ = &i2c_handles[int(config.periph)];
    return pimpl_->Init(config);
}

const I2CHandle::Config& I2CHandle::GetConfig() const
{
    return pimpl_->GetConfig();
}

I2CHandle::Result I2CHandle::TransmitBlocking(uint16_t address,
                                              uint8_t* data,
                                              uint16_t size,
                                              uint32_t timeout)
{
    return pimpl_->TransmitBlocking(address, data, size, timeout);
}

I2CHandle::Result I2CHandle::ReceiveBlocking(uint16_t address,
                                             uint8_t* data,
                                             uint16_t size,
                                             uint32_t timeout)
{
    return pimpl_->ReceiveBlocking(address, data, size, timeout);
}

I2CHandle::Result
I2CHandle::TransmitDma(uint16_t                       address,
                       uint8_t*                       data,
                       uint16_t                       size,
                       I2CHandle::CallbackFunctionPtr callback,
                       void*                          callback_context)
{
    return pimpl_->TransmitDma(address, data, size, callback, callback_context);
}

I2CHandle::Result I2CHandle::ReceiveDma(uint16_t                       address,
                                        uint8_t*                       data,
                                        uint16_t                       size,
                                        I2CHandle::CallbackFunctionPtr callback,
                                        void* callback_context)
{
    return pimpl_->ReceiveDma(address, data, size, callback, callback_context);
}


I2CHandle::Result I2CHandle::ReadDataAtAddress(uint16_t address,
                                               uint16_t mem_address,
                                               uint16_t mem_address_size,
                                               uint8_t* data,
                                               uint16_t data_size,
                                               uint32_t timeout)
{
    return pimpl_->ReadDataAtAddress(
        address, mem_address, mem_address_size, data, data_size, timeout);
}

I2CHandle::Result I2CHandle::WriteDataAtAddress(uint16_t address,
                                                uint16_t mem_address,
                                                uint16_t mem_address_size,
                                                uint8_t* data,
                                                uint16_t data_size,
                                                uint32_t timeout)
{
    return pimpl_->WriteDataAtAddress(
        address, mem_address, mem_address_size, data, data_size, timeout);
}

} // namespace daisy
