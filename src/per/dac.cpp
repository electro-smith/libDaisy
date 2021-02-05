#include "sys/system.h"
#include "per/gpio.h"
#include "per/tim.h"
#include "per/dac.h"

extern "C"
{
#include "util/hal_map.h"
}

namespace daisy
{
/** Private Implementation class */
class DacHandle::Impl
{
  public:
    DacHandle::Result        Init(const DacHandle::Config &config);
    const DacHandle::Config &GetConfig() const { return config_; }
    DacHandle::Result
                      Start(uint16_t *buffer, size_t size, DacHandle::DacCallback cb);
    DacHandle::Result Start(uint16_t *             buffer_1,
                            uint16_t *             buffer_2,
                            size_t                 size,
                            DacHandle::DacCallback cb);
    DacHandle::Result Stop();
    DacHandle::Result WriteValue(DacHandle::Channel chn, uint16_t val);


    // Init Hardware (called from MspInit, etc.)
    void InitPins();
    void DeInitPins();
    void InitDma();

    void InternalCalllback(Channel chn, size_t offset_state);

    inline bool ChannelOneActive() const
    {
        return config_.chn == Channel::BOTH || config_.chn == Channel::ONE;
    }

    inline bool ChannelTwoActive() const
    {
        return config_.chn == Channel::BOTH || config_.chn == Channel::TWO;
    }

    DAC_HandleTypeDef hal_dac_; /**< ST HAL DAC Handle*/
    DMA_HandleTypeDef
                      hal_dac_dma_[2]; /**< ST HAL DMA Hande (one per available channel) */
    TIM_HandleTypeDef hal_tim_;

  private:
    DacHandle::Config      config_; /**< Config Struct for initialization */
    size_t                 buff_size_;
    uint16_t *             buff_[2];
    DacHandle::DacCallback callback_;
};

// ================================================================
// Generic Error Handler
// ================================================================

static void ErrorHandler()
{
    while(1)
    {
#ifdef DEBUG
        asm("bkpt 255");
#endif
    }
}

// ================================================================
// Static Reference for available DAC peripheral
// ================================================================

static DacHandle::Impl dac_handle;

// ================================================================
// DAC Functions
// ================================================================

DacHandle::Result DacHandle::Impl::Init(const DacHandle::Config &config)
{
    DAC_ChannelConfTypeDef dac_config;
    config_ = config;
    // Init the actual peripheral
    hal_dac_.Instance = DAC1;
    HAL_DAC_Init(&hal_dac_);

    // Not using S-H mode for now.
    dac_config.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;

    // Configure Trig source for DAC
    dac_config.DAC_Trigger = config_.mode == Mode ::POLLING
                                 ? DAC_TRIGGER_SOFTWARE
                                 : DAC_TRIGGER_T6_TRGO;

    // Configure TIM6 for DMA Mode
    if(config_.mode == Mode::DMA)
    {
        uint32_t                tim_base_freq, target_freq, period;
        TIM_MasterConfigTypeDef tim_master_config = {0};
        hal_tim_.Instance                         = TIM6;
        hal_tim_.Init.CounterMode                 = TIM_COUNTERMODE_UP;
        hal_tim_.Init.ClockDivision               = TIM_CLOCKDIVISION_DIV1;
        hal_tim_.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

        // 16-bit values that can be used to reach target frequency.
        // Base tim freq is PClk2 * 2 (200/240MHz depending on system configuration).
        tim_base_freq = System::
            GetPClk2Freq(); // 100MHz (or 120MHz) depending on CPU Freq.
        target_freq = config_.target_samplerate == 0
                          ? 48000
                          : config_.target_samplerate;
        period                  = tim_base_freq / target_freq;
        hal_tim_.Init.Period    = period;
        hal_tim_.Init.Prescaler = 1;

        if(HAL_TIM_Base_Init(&hal_tim_) != HAL_OK)
            return Result::ERR;
        tim_master_config.MasterOutputTrigger = TIM_TRGO_UPDATE;
        tim_master_config.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
        if(HAL_TIMEx_MasterConfigSynchronization(&hal_tim_, &tim_master_config)
           != HAL_OK)
            return Result::ERR;
    }

    dac_config.DAC_OutputBuffer = config_.buff_state == BufferState::ENABLED
                                      ? DAC_OUTPUTBUFFER_ENABLE
                                      : DAC_OUTPUTBUFFER_DISABLE;

    dac_config.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
    dac_config.DAC_UserTrimming            = DAC_TRIMMING_FACTORY;
    //  Configure the channel(s) in config.
    if(ChannelOneActive())
    {
        // Init Channel 1
        if(HAL_DAC_ConfigChannel(&hal_dac_, &dac_config, DAC_CHANNEL_1)
           != HAL_OK)
        {
            return Result::ERR;
        }
    }
    if(ChannelTwoActive())
    {
        // Init Channel 2
        if(HAL_DAC_ConfigChannel(&hal_dac_, &dac_config, DAC_CHANNEL_2)
           != HAL_OK)
        {
            return Result::ERR;
        }
    }
    return Result::OK;
}

DacHandle::Result
DacHandle::Impl::Start(uint16_t *buffer, size_t size, DacHandle::DacCallback cb)
{
    if(config_.mode != Mode::DMA || config_.chn == Channel::BOTH)
        return Result::ERR;
    callback_   = cb;
    buff_[0]    = buffer;
    buff_size_  = size;
    uint32_t bd = config_.bitdepth == BitDepth::BITS_8 ? DAC_ALIGN_8B_R
                                                       : DAC_ALIGN_12B_R;
    uint32_t chn = config_.chn == Channel::ONE ? DAC_CHANNEL_1 : DAC_CHANNEL_2;
    HAL_DAC_Start_DMA(&hal_dac_, chn, (uint32_t *)buff_[0], size, bd);
    HAL_TIM_Base_Start(&hal_tim_);
    return Result::OK;
}

/** Not fully implemented -- I had the intention of setting up both DACs to work from a single callback, which 
 ** it seems like will require a bit more setup in the Init (basically set it to dual mode). */
DacHandle::Result DacHandle::Impl::Start(uint16_t *             buffer_1,
                                         uint16_t *             buffer_2,
                                         size_t                 size,
                                         DacHandle::DacCallback cb)
{
    uint32_t bd;

    // Fail if not DMA and not BOTH channels
    if(config_.mode != Mode::DMA && config_.chn != Channel::BOTH)
        return Result::ERR;

    // Set Up Buffers and Sizes
    callback_  = cb;
    buff_[0]   = buffer_1;
    buff_[1]   = buffer_2;
    buff_size_ = size;
    bd         = config_.bitdepth == BitDepth::BITS_8 ? DAC_ALIGN_8B_R
                                              : DAC_ALIGN_12B_R;

    // Start the DACs and then start the time source.
    HAL_DAC_Start_DMA(&hal_dac_, DAC_CHANNEL_1, (uint32_t *)buff_[0], size, bd);
    HAL_DAC_Start_DMA(&hal_dac_, DAC_CHANNEL_2, (uint32_t *)buff_[1], size, bd);
    HAL_TIM_Base_Start(&hal_tim_);

    return Result::OK;
}

DacHandle::Result DacHandle::Impl::Stop()
{
    if(config_.mode != Mode::DMA)
        return Result::ERR;
    if(ChannelOneActive())
        HAL_DAC_Stop_DMA(&hal_dac_, DAC_CHANNEL_1);
    if(ChannelTwoActive())
        HAL_DAC_Stop_DMA(&hal_dac_, DAC_CHANNEL_2);
    return Result::OK;
}

DacHandle::Result DacHandle::Impl::WriteValue(DacHandle::Channel chn,
                                              uint16_t           val)
{
    // This shouldn't do anything if the DAC is setup for DMA
    if(config_.mode != Mode::POLLING)
        return Result::ERR;

    // Set Value, bitdepth
    uint32_t bd = config_.bitdepth == BitDepth::BITS_8 ? DAC_ALIGN_8B_R
                                                       : DAC_ALIGN_12B_R;

    if(chn == Channel::ONE || chn == Channel::BOTH)
    {
        HAL_DAC_SetValue(&hal_dac_, DAC_CHANNEL_1, bd, val);
        HAL_DAC_Start(&hal_dac_, DAC_CHANNEL_1);
    }
    if(chn == Channel::TWO || chn == Channel::BOTH)
    {
        HAL_DAC_SetValue(&hal_dac_, DAC_CHANNEL_2, bd, val);
        HAL_DAC_Start(&hal_dac_, DAC_CHANNEL_2);
    }
    return Result::OK;
}

void DacHandle::Impl::InitPins()
{
    if(ChannelOneActive())
    {
        // Init PA4 as Analog
        dsy_gpio d1;
        d1.pin  = {DSY_GPIOA, 4};
        d1.mode = DSY_GPIO_MODE_ANALOG;
        dsy_gpio_init(&d1);
    }

    if(ChannelTwoActive())
    {
        // Init PA5 as Analog
        dsy_gpio d2;
        d2.pin  = {DSY_GPIOA, 5};
        d2.mode = DSY_GPIO_MODE_ANALOG;
        dsy_gpio_init(&d2);
    }
}

void DacHandle::Impl::InitDma()
{
    // TODO:
    // Add DMA Init for DMA2_Stream0 and DMA2_Stream1 to sys/dma init
    if(config_.mode == Mode::DMA)
    {
        // Initialize the DMA for the selected channel(s)
        DMA_HandleTypeDef *hdma;
        if(ChannelOneActive())
        {
            // Configure and Initialize channel 1 DMA
            hdma                           = &hal_dac_dma_[0];
            hdma->Instance                 = DMA2_Stream0;
            hdma->Init.Request             = DMA_REQUEST_DAC1;
            hdma->Init.Direction           = DMA_MEMORY_TO_PERIPH;
            hdma->Init.PeriphInc           = DMA_PINC_DISABLE;
            hdma->Init.MemInc              = DMA_MINC_ENABLE;
            hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
            hdma->Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
            hdma->Init.Mode                = DMA_CIRCULAR;
            hdma->Init.Priority            = DMA_PRIORITY_MEDIUM;
            hdma->Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
            if(HAL_DMA_Init(hdma) != HAL_OK)
            {
                ErrorHandler();
            }
            __HAL_LINKDMA(&hal_dac_, DMA_Handle1, hal_dac_dma_[0]);
        }
        if(ChannelTwoActive())
        {
            // Configure and Initialize channel 2 DMA
            hdma                           = &hal_dac_dma_[1];
            hdma->Instance                 = DMA2_Stream1;
            hdma->Init.Request             = DMA_REQUEST_DAC2;
            hdma->Init.Direction           = DMA_MEMORY_TO_PERIPH;
            hdma->Init.PeriphInc           = DMA_PINC_DISABLE;
            hdma->Init.MemInc              = DMA_MINC_ENABLE;
            hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
            hdma->Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
            hdma->Init.Mode                = DMA_CIRCULAR;
            hdma->Init.Priority            = DMA_PRIORITY_MEDIUM;
            hdma->Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
            if(HAL_DMA_Init(hdma) != HAL_OK)
            {
                ErrorHandler();
            }
            __HAL_LINKDMA(&hal_dac_, DMA_Handle2, hal_dac_dma_[1]);
        }
    }
}

void DacHandle::Impl::DeInitPins()
{
    if(ChannelOneActive())
    {
        dsy_gpio d1;
        d1.pin = {DSY_GPIOA, 4};
        dsy_gpio_deinit(&d1);
    }
    if(ChannelTwoActive())
    {
        dsy_gpio d2;
        d2.pin = {DSY_GPIOA, 5};
        dsy_gpio_deinit(&d2);
    }
}

void DacHandle::Impl::InternalCalllback(DacHandle::Channel channel,
                                        size_t             offset_state)
{
    uint16_t offset = offset_state ? buff_size_ / 2 : 0;
    if(config_.chn == Channel::BOTH && buff_[1] != nullptr
       && channel != Channel::TWO)
    {
        // Both channels as long as second buffer is not null.
        // We ignore IRQs from the second DAC channel since
        // both were started at the same time.
        uint16_t *buffers[2];
        buffers[0] = buff_[0] + offset;
        buffers[1] = buff_[1] + offset;
        if(callback_ != nullptr)
            (callback_)(buffers, buff_size_ / 2);
    }
    else if(config_.chn != Channel::BOTH)
    {
        int32_t   chn  = channel == Channel::TWO ? 1 : 0;
        uint16_t *buff = buff_[chn] + offset;
        uint16_t *buffers[1];
        buffers[0] = buff;
        if(callback_ != nullptr)
            (callback_)(buffers, buff_size_ / 2);
    }
}

// ======================================================================
// HAL Service Functions
// ======================================================================

extern "C" void HAL_DAC_MspInit(DAC_HandleTypeDef *hdac)
{
    if(hdac->Instance == DAC1)
    {
        __HAL_RCC_DAC12_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        // might need this for timing reasons, but does get called in tim.cpp
        //__HAL_RCC_TIM6_CLK_ENABLE();

        dac_handle.InitPins();
        dac_handle.InitDma();

        // This stuff is more relevant to TIM6, but makes more sense to be enabled here.
        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    }
}

extern "C" void HAL_DAC_MspDeInit(DAC_HandleTypeDef *hdac)
{
    if(hdac->Instance == DAC1)
    {
        __HAL_RCC_DAC12_CLK_DISABLE();
        dac_handle.DeInitPins();
    }
}

// ======================================================================
// ISRs and event handlers
// ======================================================================

extern "C" void DMA2_Stream0_IRQHandler(void)
{
    // DAC1 Ch1 IRQ Handler
    HAL_DMA_IRQHandler(&dac_handle.hal_dac_dma_[0]);
}

extern "C" void DMA2_Stream1_IRQHandler(void)
{
    // DAC1 Ch2 IRQ Handler
    HAL_DMA_IRQHandler(&dac_handle.hal_dac_dma_[1]);
}

extern "C" void TIM6_DAC_IRQHandler(void)
{
    HAL_DAC_IRQHandler(&dac_handle.hal_dac_);
    HAL_TIM_IRQHandler(&dac_handle.hal_tim_);
}

extern "C" void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    dac_handle.InternalCalllback(DacHandle::Channel::ONE, 1);
}

extern "C" void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    dac_handle.InternalCalllback(DacHandle::Channel::ONE, 0);
}

extern "C" void HAL_DACEx_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef *hdac)
{
    dac_handle.InternalCalllback(DacHandle::Channel::TWO, 0);
}
extern "C" void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef *hdac)
{
    dac_handle.InternalCalllback(DacHandle::Channel::TWO, 1);
}

//extern "C" void HAL_DAC_ConvCpltCallbackCh2(DAC_HandleTypeDef *hdac)
//{
//    dac_handle.InternalCalllback(DacHandle::Channel::TWO, 1);
//}
//
//extern "C" void HAL_DAC_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef *hdac)
//{
//    dac_handle.InternalCalllback(DacHandle::Channel::TWO, 0);
//}


// ================================================================
// DacHandle -> DacHandle::Pimpl Interface
// ================================================================

DacHandle::Result DacHandle::Init(const Config &config)
{
    pimpl_ = &dac_handle;
    return pimpl_->Init(config);
}

const DacHandle::Config &DacHandle::GetConfig() const
{
    return pimpl_->GetConfig();
}

DacHandle::Result
DacHandle::Start(uint16_t *buffer, size_t size, DacHandle::DacCallback cb)
{
    return pimpl_->Start(buffer, size, cb);
}

DacHandle::Result DacHandle::Start(uint16_t *             buffer_1,
                                   uint16_t *             buffer_2,
                                   size_t                 size,
                                   DacHandle::DacCallback cb)
{
    return pimpl_->Start(buffer_1, buffer_2, size, cb);
}

DacHandle::Result DacHandle::Stop()
{
    return pimpl_->Stop();
}

DacHandle::Result DacHandle::WriteValue(DacHandle::Channel chn, uint16_t val)
{
    return pimpl_->WriteValue(chn, val);
}


} // namespace daisy
