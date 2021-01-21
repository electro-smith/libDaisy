#include "per/dac.h"
#include "per/gpio.h"
#include "stm32h7xx_hal.h"

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
    DacHandle::Result WriteValue(DacHandle::Config::Channel chn, uint16_t val);

    // Init Hardware (called from MspInit, etc.)
    void InitPins();
    void InitDma();

  private:
    DacHandle::Config      config_;  /**< Config Struct for initialization */
    DAC_HandleTypeDef      hal_dac_; /**< ST HAL DAC Handle*/
    size_t                 buff_size_;
    uint16_t *             buff_[2];
    DacHandle::DacCallback callback_;
};

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

    // Configure global stuff.
    dac_config.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;

    // TODO:
    // actually configure TIM5 for running the DAC.
    dac_config.DAC_Trigger = config_.mode == Config::Mode ::POLLING
                                 ? DAC_TRIGGER_SOFTWARE
                                 : DAC_TRIGGER_T5_TRGO;

    dac_config.DAC_OutputBuffer
        = config_.buff_state == Config::BufferState::ENABLED
              ? DAC_OUTPUTBUFFER_ENABLE
              : DAC_OUTPUTBUFFER_DISABLE;

    dac_config.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
    dac_config.DAC_UserTrimming            = DAC_TRIMMING_FACTORY;
    //  Configure the channel(s) in config.
    if(config.chn == Config::Channel::ONE
       || config.chn == Config::Channel::BOTH)
    {
        // Init Channel 1
        if(HAL_DAC_ConfigChannel(&hal_dac_, &dac_config, DAC_CHANNEL_1)
           != HAL_OK)
        {
            return Result::ERR;
        }
    }
    if(config.chn == Config::Channel::TWO
       || config.chn == Config::Channel::BOTH)
    {
        // Init Channel 2
        if(HAL_DAC_ConfigChannel(&hal_dac_, &dac_config, DAC_CHANNEL_1)
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
    return Result::OK;
}

DacHandle::Result DacHandle::Impl::Start(uint16_t *             buffer_1,
                                         uint16_t *             buffer_2,
                                         size_t                 size,
                                         DacHandle::DacCallback cb)
{
    return Result::OK;
}

DacHandle::Result DacHandle::Impl::Stop()
{
    return Result::OK;
}

DacHandle::Result DacHandle::Impl::WriteValue(DacHandle::Config::Channel chn,
                                              uint16_t                   val)
{
    // This shouldn't do anything if the DAC is setup for DMA
    if(config_.mode != Config::Mode::POLLING)
        return Result::ERR;

    // Set Value, bitdepth
    uint32_t bd = config_.bitdepth == Config::BitDepth::BITS_8
                      ? DAC_ALIGN_8B_R
                      : DAC_ALIGN_12B_R;

    if(chn == Config::Channel::BOTH || chn == Config::Channel::ONE)
        HAL_DAC_SetValue(&hal_dac_, DAC_CHANNEL_1, bd, val);
    if(chn == Config::Channel::BOTH || chn == Config::Channel::TWO)
        HAL_DAC_SetValue(&hal_dac_, DAC_CHANNEL_2, bd, val);

    // Start the DAC
    switch(config_.chn)
    {
        case Config::Channel::ONE:
            HAL_DAC_Start(&hal_dac_, DAC_CHANNEL_1);
            break;
        case Config::Channel::TWO:
            HAL_DAC_Start(&hal_dac_, DAC_CHANNEL_2);
            break;
        case Config::Channel::BOTH:
            HAL_DAC_Start(&hal_dac_, DAC_CHANNEL_1);
            HAL_DAC_Start(&hal_dac_, DAC_CHANNEL_2);
            break;
    }
    return Result::OK;
}

void DacHandle::Impl::InitPins()
{
    if(dac_handle.GetConfig().chn == DacHandle::Config::Channel::BOTH
       || dac_handle.GetConfig().chn == DacHandle::Config::Channel::ONE)
    {
        // Init PA4 as Analog
        dsy_gpio d1;
        d1.pin  = {DSY_GPIOA, 4};
        d1.mode = DSY_GPIO_MODE_ANALOG;
        dsy_gpio_init(&d1);
    }

    if(dac_handle.GetConfig().chn == DacHandle::Config::Channel::BOTH
       || dac_handle.GetConfig().chn == DacHandle::Config::Channel::TWO)
    {
        // Init PA5 as Analog
        dsy_gpio d2;
        d2.pin  = {DSY_GPIOA, 5};
        d2.mode = DSY_GPIO_MODE_ANALOG;
        dsy_gpio_init(&d2);
    }

}

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

DacHandle::Result DacHandle::WriteValue(DacHandle::Config::Channel chn,
                                        uint16_t                   val)
{
    return pimpl_->WriteValue(chn, val);
}

extern "C" void HAL_DAC_MspInit(DAC_HandleTypeDef *hdac)
{
    GPIO_InitTypeDef ginit;
    GPIO_TypeDef *   port;
    dsy_gpio_pin *   p;
    if(hdac->Instance == DAC1)
    {
        __HAL_RCC_DAC12_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        ginit.Mode = GPIO_MODE_ANALOG;
        ginit.Pull = GPIO_NOPULL;
        dac_handle.InitPins();
        //        if(dac.active_channels == DSY_DAC_CHN1
        //           || dac.active_channels == DSY_DAC_CHN_BOTH)
        //        {
        //            p         = &dac.dsy_hdac->pin_config[DSY_DAC_CHN1];
        //            port      = dsy_hal_map_get_port(p);
        //            ginit.Pin = dsy_hal_map_get_pin(p);
        //            HAL_GPIO_Init(port, &ginit);
        //        }
        //        if(dac.active_channels == DSY_DAC_CHN2
        //           || dac.active_channels == DSY_DAC_CHN_BOTH)
        //        {
        //            p         = &dac.dsy_hdac->pin_config[DSY_DAC_CHN2];
        //            port      = dsy_hal_map_get_port(p);
        //            ginit.Pin = dsy_hal_map_get_pin(p);
        //            HAL_GPIO_Init(port, &ginit);
        //        }
    }
}

extern "C" void HAL_DAC_MspDeInit(DAC_HandleTypeDef *hdac)
{
    GPIO_TypeDef *port;
    uint16_t      pin;
    dsy_gpio_pin *p;
    if(hdac->Instance == DAC1)
    {
        __HAL_RCC_DAC12_CLK_DISABLE();
        //        if(dac.active_channels == DSY_DAC_CHN1
        //           || dac.active_channels == DSY_DAC_CHN_BOTH)
        //        {
        //            p    = &dac.dsy_hdac->pin_config[DSY_DAC_CHN1];
        //            port = dsy_hal_map_get_port(p);
        //            pin  = dsy_hal_map_get_pin(p);
        //            HAL_GPIO_DeInit(port, pin);
        //        }
        //        if(dac.active_channels == DSY_DAC_CHN2
        //           || dac.active_channels == DSY_DAC_CHN_BOTH)
        //        {
        //            p    = &dac.dsy_hdac->pin_config[DSY_DAC_CHN2];
        //            port = dsy_hal_map_get_port(p);
        //            pin  = dsy_hal_map_get_pin(p);
        //            HAL_GPIO_DeInit(port, pin);
        //        }
        //    }
    }
}


} // namespace daisy


// OLD CODE BELOW

typedef struct
{
    DAC_HandleTypeDef hdac1; // ST HAL DAC handle
    dsy_dac_handle *  dsy_hdac;
    uint8_t           initialized;
    dsy_dac_channel   active_channels;
} dsy_dac_t;

static dsy_dac_t dac;

void dsy_dac_init(dsy_dac_handle *dsy_hdac, dsy_dac_channel channel)
{
    DAC_ChannelConfTypeDef dac_config;
    dac.dsy_hdac = dsy_hdac;
    if(!dac.initialized)
    {
        // Init the actual peripheral
        dac.hdac1.Instance = DAC1;
        HAL_DAC_Init(&dac.hdac1);
        dac.initialized = 1;
    }
    // Setup common config
    dac_config.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
    dac_config.DAC_Trigger = DAC_TRIGGER_SOFTWARE; // TODO: Set to timer base
    dac_config.DAC_OutputBuffer            = DAC_OUTPUTBUFFER_ENABLE;
    dac_config.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
    dac_config.DAC_UserTrimming            = DAC_TRIMMING_FACTORY;
    if(channel == DSY_DAC_CHN1 || channel == DSY_DAC_CHN_BOTH)
    {
        // Init Channel 1
        if(HAL_DAC_ConfigChannel(&dac.hdac1, &dac_config, DAC_CHANNEL_1)
           != HAL_OK)
        {
            // Fail
        }
    }
    if(channel == DSY_DAC_CHN2 || channel == DSY_DAC_CHN_BOTH)
    {
        // Init Channel 2
        if(HAL_DAC_ConfigChannel(&dac.hdac1, &dac_config, DAC_CHANNEL_2)
           != HAL_OK)
        {
            // Fail
        }
    }
    dac.active_channels = channel;
}

void dsy_dac_start(dsy_dac_channel channel)
{
    switch(channel)
    {
        case DSY_DAC_CHN1: HAL_DAC_Start(&dac.hdac1, DAC_CHANNEL_1); break;
        case DSY_DAC_CHN2: HAL_DAC_Start(&dac.hdac1, DAC_CHANNEL_2); break;
        case DSY_DAC_CHN_BOTH:
            HAL_DAC_Start(&dac.hdac1, DAC_CHANNEL_1);
            HAL_DAC_Start(&dac.hdac1, DAC_CHANNEL_2);
            break;
        default: break;
    }
}
void dsy_dac_write(dsy_dac_channel channel, uint16_t val)
{
    uint8_t bitdepth;
    if(dac.dsy_hdac->bitdepth == DSY_DAC_BITS_8)
    {
        bitdepth = DAC_ALIGN_8B_R;
    }
    else
    {
        bitdepth = DAC_ALIGN_12B_R;
    }
    switch(channel)
    {
        case DSY_DAC_CHN1:
            HAL_DAC_SetValue(&dac.hdac1, DAC_CHANNEL_1, bitdepth, val);
            break;
        case DSY_DAC_CHN2:
            HAL_DAC_SetValue(&dac.hdac1, DAC_CHANNEL_2, bitdepth, val);
            break;
        case DSY_DAC_CHN_BOTH:
            HAL_DAC_SetValue(&dac.hdac1, DAC_CHANNEL_1, bitdepth, val);
            HAL_DAC_SetValue(&dac.hdac1, DAC_CHANNEL_2, bitdepth, val);
            break;
        default: break;
    }
    // If there's a timer running, this isn't necessary.
    if(dac.dsy_hdac->mode == DSY_DAC_MODE_POLLING)
    {
        dsy_dac_start(channel);
    }
}

////} // namespace daisy
