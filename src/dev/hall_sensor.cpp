#include "hall_sensor.h"

namespace daisy
{
void HallSensor::Init(Config config)
{
    config_ = config;

    TIM_HallSensor_InitTypeDef hall_conf_;

    hall_conf_.IC1Polarity       = config_.polarity;
    hall_conf_.IC1Prescaler      = config_.prescaler;
    hall_conf_.IC1Filter         = config_.filter;
    hall_conf_.Commutation_Delay = config_.commutation_delay;

    HAL_TIMEx_HallSensor_Init(&hall_, &hall_conf_);

    StartBlockingRead(); // blocking read by default
}

void HallSensor::StartBlockingRead()
{
    // are we ready, kids?
    while(HAL_TIMEx_HallSensor_GetState(&hall_) != HAL_TIM_STATE_READY) {};

    HAL_TIMEx_HallSensor_Start(&hall_);
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
#endif