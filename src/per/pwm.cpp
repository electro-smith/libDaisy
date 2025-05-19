#include "pwm.h"
#include "stm32h7xx_hal.h"
#include "util/hal_map.h"

namespace daisy
{
class PWMHandle::Impl
{
  public:
    PWMHandle::Result Init(const Config &config);
    PWMHandle::Result DeInit();
    void              SetPrescaler(uint32_t prescaler);
    void              SetPeriod(uint32_t period);

    Config            config_;
    TIM_HandleTypeDef tim_hal_handle_{0};
};

#define NUM_TIMERS 3

// Static storage for implementations
static PWMHandle::Impl pwm_handles[NUM_TIMERS];

// ---------------------------------------------

Pin GetDefaultPin(PWMHandle::Config::Peripheral timer, uint32_t channel)
{
    if(timer == PWMHandle::Config::Peripheral::TIM_3)
    {
        switch(channel)
        {
            case TIM_CHANNEL_1: return {PORTA, 6};
            case TIM_CHANNEL_2: return {PORTC, 7};
            case TIM_CHANNEL_3: return {PORTC, 8};
            case TIM_CHANNEL_4: return {PORTB, 1};
            default: break;
        }
    }

    if(timer == PWMHandle::Config::Peripheral::TIM_4)
    {
        switch(channel)
        {
            case TIM_CHANNEL_1: return {PORTB, 6};
            case TIM_CHANNEL_2: return {PORTB, 7};
            case TIM_CHANNEL_3: return {PORTB, 8};
            case TIM_CHANNEL_4: return {PORTB, 9};
            default: break;
        }
    }

    if(timer == PWMHandle::Config::Peripheral::TIM_5)
    {
        switch(channel)
        {
            case TIM_CHANNEL_1: return {PORTA, 0};
            case TIM_CHANNEL_2: return {PORTA, 1};
            case TIM_CHANNEL_3: return {PORTA, 2};
            case TIM_CHANNEL_4: return {PORTA, 3};
            default: break;
        }
    }

    return Pin();
}

PWMHandle::Result
PWMHandle::Channel::Init(const PWMHandle::Channel::Config &config)
{
    if(owner_.pimpl_ == nullptr)
        return PWMHandle::Result::ERR;

    config_ = config;
    handle_ = &(owner_.pimpl_->tim_hal_handle_);

    // float multiplier
    scale_ = static_cast<float>(owner_.pimpl_->config_.period);

    // Configure channel
    TIM_OC_InitTypeDef oc_config = {0};
    oc_config.OCMode             = TIM_OCMODE_PWM1;
    oc_config.Pulse              = 0;
    oc_config.OCPolarity
        = (config_.polarity == PWMHandle::Channel::Config::Polarity::LOW)
              ? TIM_OCPOLARITY_LOW
              : TIM_OCPOLARITY_HIGH;
    oc_config.OCFastMode = TIM_OCFAST_DISABLE;
    if(HAL_TIM_PWM_ConfigChannel(handle_, &oc_config, channel_) != HAL_OK)
    {
        return PWMHandle::Result::ERR;
    }

    // Configure pin
    if(!config_.pin.IsValid())
    {
        config_.pin = GetDefaultPin(owner_.pimpl_->config_.periph, channel_);
    }
    if(!config_.pin.IsValid())
    {
        return PWMHandle::Result::ERR;
    }
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_TypeDef *   GPIO_Port       = GetHALPort(config_.pin);

    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    switch(owner_.pimpl_->config_.periph)
    {
        case PWMHandle::Config::Peripheral::TIM_3:
            GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
            break;
        case PWMHandle::Config::Peripheral::TIM_4:
            GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
            break;
        case PWMHandle::Config::Peripheral::TIM_5:
            GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
            break;
    }

    GPIO_InitStruct.Pin = GetHALPin(config_.pin);

    switch(config_.pin.port)
    {
        case PORTA: __HAL_RCC_GPIOA_CLK_ENABLE(); break;
        case PORTB: __HAL_RCC_GPIOB_CLK_ENABLE(); break;
        case PORTC: __HAL_RCC_GPIOC_CLK_ENABLE(); break;
        case PORTD: __HAL_RCC_GPIOD_CLK_ENABLE(); break;
        case PORTE: __HAL_RCC_GPIOE_CLK_ENABLE(); break;
        case PORTF: __HAL_RCC_GPIOF_CLK_ENABLE(); break;
        case PORTG: __HAL_RCC_GPIOG_CLK_ENABLE(); break;
        case PORTH: __HAL_RCC_GPIOH_CLK_ENABLE(); break;
        case PORTI: __HAL_RCC_GPIOI_CLK_ENABLE(); break;
        case PORTJ: __HAL_RCC_GPIOJ_CLK_ENABLE(); break;
        case PORTK: __HAL_RCC_GPIOK_CLK_ENABLE(); break;
        default: break;
    }

    HAL_GPIO_Init(GPIO_Port, &GPIO_InitStruct);

    // Start PWM on channel
    if(HAL_TIM_PWM_Start(handle_, channel_) != HAL_OK)
    {
        return PWMHandle::Result::ERR;
    }

    return PWMHandle::Result::OK;
}

PWMHandle::Result PWMHandle::Channel::Init()
{
    return Init({});
}

PWMHandle::Result PWMHandle::Channel::DeInit()
{
    if(handle_ == nullptr)
        return PWMHandle::Result::OK;

    auto *handle = &(owner_.pimpl_->tim_hal_handle_);
    if(HAL_TIM_PWM_Stop(handle, channel_) != HAL_OK)
    {
        return PWMHandle::Result::ERR;
    }

    handle_ = nullptr;

    return PWMHandle::Result::OK;
}


// -------------------------------------------------------------------------

PWMHandle::Result PWMHandle::Impl::Init(const PWMHandle::Config &config)
{
    config_ = config;

    // TIM3 and TIM4 are 16-bit; TIM5 is 32-bit
    switch(config_.periph)
    {
        case PWMHandle::Config::Peripheral::TIM_3:
        case PWMHandle::Config::Peripheral::TIM_4:
            config_.period = (uint16_t)config_.period;
            break;
        default: break;
    }

    // Build TIM handle
    constexpr TIM_TypeDef *pwm_handles[NUM_TIMERS] = {TIM3, TIM4, TIM5};
    tim_hal_handle_.Instance = pwm_handles[static_cast<size_t>(config.periph)];
    tim_hal_handle_.Init.CounterMode       = TIM_COUNTERMODE_UP;
    tim_hal_handle_.Init.Prescaler         = config_.prescaler;
    tim_hal_handle_.Init.Period            = config_.period;
    tim_hal_handle_.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    tim_hal_handle_.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    // Enable timer clock
    switch(config_.periph)
    {
        case PWMHandle::Config::Peripheral::TIM_3:
            __HAL_RCC_TIM3_CLK_ENABLE();
            break;

        case PWMHandle::Config::Peripheral::TIM_4:
            __HAL_RCC_TIM4_CLK_ENABLE();
            break;

        case PWMHandle::Config::Peripheral::TIM_5:
            __HAL_RCC_TIM5_CLK_ENABLE();
            break;
    }

    // Init timer
    if(HAL_TIM_Base_Init(&tim_hal_handle_) != HAL_OK)
    {
        return PWMHandle::Result::ERR;
    }

    // Set clock source to internal
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    sClockSourceConfig.ClockSource            = TIM_CLOCKSOURCE_INTERNAL;
    if(HAL_TIM_ConfigClockSource(&tim_hal_handle_, &sClockSourceConfig)
       != HAL_OK)
    {
        return PWMHandle::Result::ERR;
    }

    // Init PWM
    if(HAL_TIM_PWM_Init(&tim_hal_handle_) != HAL_OK)
    {
        return PWMHandle::Result::ERR;
    }

    // Set synchronization config
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    sMasterConfig.MasterOutputTrigger     = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode         = TIM_MASTERSLAVEMODE_DISABLE;
    if(HAL_TIMEx_MasterConfigSynchronization(&tim_hal_handle_, &sMasterConfig)
       != HAL_OK)
    {
        return PWMHandle::Result::ERR;
    }

    return PWMHandle::Result::OK;
}

PWMHandle::Result PWMHandle::Impl::DeInit()
{
    if(HAL_TIM_PWM_DeInit(&tim_hal_handle_) != HAL_OK)
    {
        return PWMHandle::Result::ERR;
    }

    if(HAL_TIM_Base_DeInit(&tim_hal_handle_) != HAL_OK)
    {
        return PWMHandle::Result::ERR;
    }

    return PWMHandle::Result::OK;
}

void PWMHandle::Impl::SetPrescaler(uint32_t prescaler)
{
    config_.prescaler = prescaler;
    __HAL_TIM_SET_PRESCALER(&tim_hal_handle_, prescaler);
}

void PWMHandle::Impl::SetPeriod(uint32_t period)
{
    config_.period = period;
    __HAL_TIM_SET_AUTORELOAD(&tim_hal_handle_, period);
}

// ---------------------------------------------------------------------

PWMHandle::PWMHandle()
: pimpl_(nullptr),
  ch1_(this, TIM_CHANNEL_1),
  ch2_(this, TIM_CHANNEL_2),
  ch3_(this, TIM_CHANNEL_3),
  ch4_(this, TIM_CHANNEL_4)
{
}

PWMHandle::Result PWMHandle::Init(const PWMHandle::Config &config)
{
    const int pwm_idx = static_cast<size_t>(config.periph);
    if(pwm_idx >= NUM_TIMERS)
        return PWMHandle::Result::ERR;

    pimpl_ = &pwm_handles[pwm_idx];
    return pimpl_->Init(config);
}

PWMHandle::Result PWMHandle::DeInit()
{
    int result = 0;

    result |= (int)ch1_.DeInit();
    result |= (int)ch2_.DeInit();
    result |= (int)ch3_.DeInit();
    result |= (int)ch4_.DeInit();
    result |= (int)pimpl_->DeInit();

    return static_cast<PWMHandle::Result>(result);
}

const PWMHandle::Config &PWMHandle::GetConfig() const
{
    return pimpl_->config_;
}

void PWMHandle::SetPrescaler(uint32_t prescaler)
{
    pimpl_->SetPrescaler(prescaler);
}

void PWMHandle::SetPeriod(uint32_t period)
{
    pimpl_->SetPeriod(period);
}

} // namespace daisy