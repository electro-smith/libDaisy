#include "tim_channel.h"
#include "util/hal_map.h"

namespace daisy
{
static DMA_HandleTypeDef timhdma;

/** Pin Mappings:
     *  TODO: Make a map
     * 
     *  TIM2 CH1 - PA0 (AF1), PA5 (AF1)
     *  TIM2 CH2 - PA1 (AF1), PB3 (AF1)
     *  TIM2 CH3 - PA2 (AF1), PB10 (AF1)
     *  TIM2 CH4 - PA3 (AF1), PB11 (AF1)
     *  TIM3 CH1 - PA6 (AF2), PB4 (AF2), PC6 (AF2)
     *  TIM3 CH2 - PA7 (AF2), PB5 (AF2), PC7 (AF2)
     *  TIM3 CH3 - PB0 (AF2), PC8 (AF2)
     *  TIM3 CH4 - PB1 (AF2), PC9 (AF2)
     *  TIM4 CH1 - PD12 (AF2), PB6 (AF2)
     *  TIM4 CH2 - PD13 (AF2), PB7 (AF2)
     *  TIM4 CH3 - PD14 (AF2), PB8 (AF2)
     *  TIM4 CH4 - PD15 (AF2), PB9 (AF2)
     *  TIM5 CH1 - PA0 (AF2), PH10 (AF2)
     *  TIM5 CH2 - PA1 (AF2), PH11 (AF2)
     *  TIM5 CH3 - PA2 (AF2), PH12 (AF2)
     *  TIM5 CH4 - PA3 (AF2), PI0 (AF2)
     * 
     *  And without a map:
     *  TIM2 ChN (AF1)
     *  TIM3 ChN (AF2)
     *  TIM4 ChN (AF2)
     *  TIM5 ChN (AF2)
    */

/** Sets the instance of the HAL TIM Handle based on the values in the Daisy struct 
 *  This also returns the AF value for a given timer (can be used for GPIO init).
 */
static uint32_t SetInstance(TIM_HandleTypeDef*              tim,
                            TimerHandle::Config::Peripheral dsy_periph)
{
    uint32_t af_value;
    switch(dsy_periph)
    {
        case TimerHandle::Config::Peripheral::TIM_2:
            tim->Instance = TIM2;
            af_value      = GPIO_AF1_TIM2;
            break;
        case TimerHandle::Config::Peripheral::TIM_3:
            tim->Instance = TIM3;
            af_value      = GPIO_AF2_TIM3;
            break;
        case TimerHandle::Config::Peripheral::TIM_4:
            tim->Instance = TIM4;
            af_value      = GPIO_AF2_TIM4;
            break;
        case TimerHandle::Config::Peripheral::TIM_5:
            tim->Instance = TIM5;
            af_value      = GPIO_AF2_TIM5;
            break;
    }
    return af_value;
}

static uint32_t GetHalChannel(TimChannel::Config::Channel chn)
{
    auto hal_chn = chn == TimChannel::Config::Channel::ONE     ? TIM_CHANNEL_1
                   : chn == TimChannel::Config::Channel::TWO   ? TIM_CHANNEL_2
                   : chn == TimChannel::Config::Channel::THREE ? TIM_CHANNEL_3
                   : chn == TimChannel::Config::Channel::FOUR  ? TIM_CHANNEL_4
                                                               : TIM_CHANNEL_1;
    return hal_chn;
}

void TimChannel::Init(const TimChannel::Config& cfg)
{
    cfg_ = cfg;
    /** Configure Channel */
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode             = TIM_OCMODE_PWM1;
    sConfigOC.Pulse              = 0;
    sConfigOC.OCPolarity         = cfg.polarity == Config::Polarity::HIGH
                                       ? TIM_OCPOLARITY_HIGH
                                       : TIM_OCPOLARITY_LOW;
    sConfigOC.OCFastMode         = TIM_OCFAST_DISABLE;
    auto              chn        = GetHalChannel(cfg.chn);
    TIM_HandleTypeDef tim;
    auto              af_value = SetInstance(&tim, cfg.tim->GetConfig().periph);
    HAL_TIM_PWM_ConfigChannel(&tim, &sConfigOC, chn);

    /** TODO: remove conversion to old pin, and add hal map for new Pin type */
    dsy_gpio_pin  tpin = cfg.pin;
    GPIO_TypeDef* port = dsy_hal_map_get_port(&tpin);
    uint16_t      pin  = dsy_hal_map_get_pin(&tpin);
    /** Start Clock for port (if necessary) */
    dsy_hal_map_gpio_clk_enable(tpin.port);
    /** Intilize the actual pin */
    GPIO_InitTypeDef gpio_init = {0};
    gpio_init.Pin              = pin;
    gpio_init.Mode             = GPIO_MODE_AF_PP;
    gpio_init.Pull             = GPIO_NOPULL;
    gpio_init.Speed            = GPIO_SPEED_LOW;
    gpio_init.Alternate        = af_value;
    HAL_GPIO_Init(port, &gpio_init);
}

const TimChannel::Config& TimChannel::GetConfig() const
{
    return cfg_;
}

void TimChannel::Start()
{
    TIM_HandleTypeDef tim;
    SetInstance(&tim, cfg_.tim->GetConfig().periph);
    HAL_TIM_PWM_Start(&tim, GetHalChannel(cfg_.chn));
}
void TimChannel::Stop()
{
    TIM_HandleTypeDef tim;
    SetInstance(&tim, cfg_.tim->GetConfig().periph);
    HAL_TIM_PWM_Stop(&tim, GetHalChannel(cfg_.chn));
}

void TimChannel::SetPwm(uint32_t val)
{
    TIM_HandleTypeDef tim;
    SetInstance(&tim, cfg_.tim->GetConfig().periph);
    __HAL_TIM_SET_COMPARE(&tim, GetHalChannel(cfg_.chn), val);
}
static TIM_HandleTypeDef               globaltim;
TimChannel::EndTransmissionFunctionPtr globalcb;
void*                                  globalcb_context;

void TimChannel::StartDma(void*                                  data,
                          size_t                                 size,
                          TimChannel::EndTransmissionFunctionPtr callback,
                          void*                                  cb_context)
{
    timhdma.Instance = DMA2_Stream5;

    globalcb         = callback;
    globalcb_context = cb_context;

    /** kind of nuts to set this.... */
    switch(cfg_.tim->GetConfig().periph)
    {
        case TimerHandle::Config::Peripheral::TIM_2:
            switch(cfg_.chn)
            {
                case TimChannel::Config::Channel::ONE:
                    timhdma.Init.Request = DMA_REQUEST_TIM2_CH1;
                    break;
                case TimChannel::Config::Channel::TWO:
                    timhdma.Init.Request = DMA_REQUEST_TIM2_CH2;
                    break;
                case TimChannel::Config::Channel::THREE:
                    timhdma.Init.Request = DMA_REQUEST_TIM2_CH3;
                    break;
                case TimChannel::Config::Channel::FOUR:
                    timhdma.Init.Request = DMA_REQUEST_TIM2_CH4;
                    break;
            }
            break;
        case TimerHandle::Config::Peripheral::TIM_3:
            switch(cfg_.chn)
            {
                case TimChannel::Config::Channel::ONE:
                    timhdma.Init.Request = DMA_REQUEST_TIM3_CH1;
                    break;
                case TimChannel::Config::Channel::TWO:
                    timhdma.Init.Request = DMA_REQUEST_TIM3_CH2;
                    break;
                case TimChannel::Config::Channel::THREE:
                    timhdma.Init.Request = DMA_REQUEST_TIM3_CH3;
                    break;
                case TimChannel::Config::Channel::FOUR:
                    timhdma.Init.Request = DMA_REQUEST_TIM3_CH4;
                    break;
            }
            break;
        case TimerHandle::Config::Peripheral::TIM_4:
            switch(cfg_.chn)
            {
                case TimChannel::Config::Channel::ONE:
                    timhdma.Init.Request = DMA_REQUEST_TIM4_CH1;
                    break;
                case TimChannel::Config::Channel::TWO:
                    timhdma.Init.Request = DMA_REQUEST_TIM4_CH2;
                    break;
                case TimChannel::Config::Channel::THREE:
                    timhdma.Init.Request = DMA_REQUEST_TIM4_CH3;
                    break;
                case TimChannel::Config::Channel::FOUR:
                    timhdma.Init.Request
                        = DMA_REQUEST_TIM4_UP; /**< TIM4_CH4 DMA Rq doesn't exist?*/
                    break;
            }
            break;
        case TimerHandle::Config::Peripheral::TIM_5:
            switch(cfg_.chn)
            {
                case TimChannel::Config::Channel::ONE:
                    timhdma.Init.Request = DMA_REQUEST_TIM5_CH1;
                    break;
                case TimChannel::Config::Channel::TWO:
                    timhdma.Init.Request = DMA_REQUEST_TIM5_CH2;
                    break;
                case TimChannel::Config::Channel::THREE:
                    timhdma.Init.Request = DMA_REQUEST_TIM5_CH3;
                    break;
                case TimChannel::Config::Channel::FOUR:
                    timhdma.Init.Request = DMA_REQUEST_TIM5_CH4;
                    break;
            }
            break;
    }

    timhdma.Init.Direction = DMA_MEMORY_TO_PERIPH;

    timhdma.Init.PeriphInc           = DMA_PINC_DISABLE;
    timhdma.Init.MemInc              = DMA_MINC_ENABLE;
    timhdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    timhdma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    timhdma.Init.Mode                = DMA_NORMAL;
    timhdma.Init.Priority            = DMA_PRIORITY_LOW;
    timhdma.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    timhdma.Init.MemBurst            = DMA_MBURST_SINGLE;
    timhdma.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    if(HAL_DMA_Init(&timhdma) != HAL_OK)
    {
        // something bad
    }
    SetInstance(&globaltim, cfg_.tim->GetConfig().periph);
    switch(cfg_.chn)
    {
        case TimChannel::Config::Channel::ONE:
            __HAL_LINKDMA(&globaltim, hdma[TIM_DMA_ID_CC1], timhdma);
            break;
        case TimChannel::Config::Channel::TWO:
            __HAL_LINKDMA(&globaltim, hdma[TIM_DMA_ID_CC2], timhdma);
            break;
        case TimChannel::Config::Channel::THREE:
            __HAL_LINKDMA(&globaltim, hdma[TIM_DMA_ID_CC3], timhdma);
            break;
        case TimChannel::Config::Channel::FOUR:
            __HAL_LINKDMA(&globaltim, hdma[TIM_DMA_ID_CC4], timhdma);
            break;
    }
    HAL_TIM_PWM_Start_DMA(
        &globaltim, GetHalChannel(cfg_.chn), (uint32_t*)data, size);
}


extern "C" void DMA2_Stream5_IRQHandler(void)
{
    // DMA_HandleTypeDef timhdma;
    // timhdma.Instance = DMA2_Stream5;
    HAL_DMA_IRQHandler(&timhdma);
}


extern "C" void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef* htim)
{
    if(globalcb)
    {
        globalcb(globalcb_context);
    }
}


extern "C" void DMAMUX1_OVR_IRQHandler(void) {}

} // namespace daisy