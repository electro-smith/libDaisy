#include "dev/hall_sensor.h"

namespace daisy
{
HallSensor::Result HallSensor::Init(HallSensor::Config config)
{
    config_ = config;

    TIM_HallSensor_InitTypeDef hall_conf_;

    hall_conf_.IC1Polarity       = config_.polarity;
    hall_conf_.IC1Prescaler      = config_.prescaler;
    hall_conf_.IC1Filter         = config_.filter;
    hall_conf_.Commutation_Delay = config_.commutation_delay;

    if(HAL_TIMEx_HallSensor_Init(&hall_, &hall_conf_) != HAL_OK)
    {
        return ERR;
    }

    // blocking read by default
    return StartBlockingRead();
}

HallSensor::Result HallSensor::StartBlockingRead()
{
    // are we ready, kids?
    while(HAL_TIMEx_HallSensor_GetState(&hall_) != HAL_TIM_STATE_READY) {};

    HAL_StatusTypeDef res = HAL_TIMEx_HallSensor_Start(&hall_);
    return res == HAL_OK ? OK : ERR;
}

void HallSensor::InitPins()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_TypeDef*    port;

    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT; // is this right?
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // what about this?
    switch(config_.periph)
    {
        case Config::Peripheral::TIM_1:
            GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
            break;
        case Config::Peripheral::TIM_2:
            GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
            break;
        case Config::Peripheral::TIM_3:
            GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
            break;
        case Config::Peripheral::TIM_4:
            GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
            break;
        case Config::Peripheral::TIM_5:
            GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
            break;
        case Config::Peripheral::TIM_6:
            GPIO_InitStruct.Alternate = GPIO_AF1_TIM16;
            break;
        default: break;
    }

    dsy_gpio_pin p      = config_.pin;
    port                = dsy_hal_map_get_port(&p);
    GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&p);
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    dsy_hal_map_gpio_clk_enable(p.port);
}

void HallSensor::DeInitPins()
{
    dsy_gpio_pin  p    = config_.pin;
    GPIO_TypeDef* port = dsy_hal_map_get_port(&p);
    uint16_t      pin  = dsy_hal_map_get_pin(&p);
    HAL_GPIO_DeInit(port, pin);
}

extern "C"
{
    void HAL_TIMEx_HallSensor_MspInit(TIM_HandleTypeDef* htim)
    {
        if(htim->Instance == TIM1)
        {
            __HAL_RCC_TIM1_CLK_ENABLE();
        }
        if(htim->Instance == TIM2)
        {
            __HAL_RCC_TIM2_CLK_ENABLE();
        }
        else if(htim->Instance == TIM3)
        {
            __HAL_RCC_TIM3_CLK_ENABLE();
        }
        else if(htim->Instance == TIM4)
        {
            __HAL_RCC_TIM4_CLK_ENABLE();
        }
        else if(htim->Instance == TIM5)
        {
            __HAL_RCC_TIM5_CLK_ENABLE();
        }
        else if(htim->Instance == TIM6)
        {
            __HAL_RCC_TIM6_CLK_ENABLE();
        }
    }

    void HAL_TIM_HallSensor_MspDeInit(TIM_HandleTypeDef* htim)
    {
        if(htim->Instance == TIM1)
        {
            __HAL_RCC_TIM1_CLK_DISABLE();
        }
        if(htim->Instance == TIM2)
        {
            __HAL_RCC_TIM2_CLK_DISABLE();
        }
        else if(htim->Instance == TIM3)
        {
            __HAL_RCC_TIM3_CLK_DISABLE();
        }
        else if(htim->Instance == TIM4)
        {
            __HAL_RCC_TIM4_CLK_DISABLE();
        }
        else if(htim->Instance == TIM5)
        {
            __HAL_RCC_TIM5_CLK_DISABLE();
        }
        else if(htim->Instance == TIM6)
        {
            __HAL_RCC_TIM6_CLK_DISABLE();
        }
    }
} // extern C

} // namespace daisy