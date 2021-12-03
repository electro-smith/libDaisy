#pragma once
#ifndef DSY_HALLSENSOR_H
#define DSY_HALLSENSOR_H

#include "stm32h7xx_hal.h"
extern "C"
{
#include "util/hal_map.h"
}

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** @brief Device support for US5881 Hall Effect Sensor
    @author beserge
    @date December 2021
*/
class HallSensor
{
  public:
    HallSensor() {}
    ~HallSensor() {}

    struct Config
    {
        enum class Peripheral
        {
            TIM_1,
            TIM_2,
            TIM_3,
            TIM_4,
            TIM_5,
            TIM_6
        };

        Peripheral periph;

        uint32_t polarity;
        uint32_t prescaler;
        uint32_t filter;
        uint32_t commutation_delay;

        dsy_gpio_pin pin_;

        Config()
        {
            // These defaults are for the US5881
            periph = Peripheral::TIM_2;
            pin_   = {DSY_GPIOX, 0};

            polarity  = TIM_ICPOLARITY_RISING; // rising, falling, bothedge
            prescaler = TIM_ICPSC_DIV1;        // 1, 2, 4, 8
            filter    = 0x0;                   // 0x0 -> 0xF
            commutation_delay = 0x0000;        // 0x0000 -> 0xFFFF
        }
    };

    /** Initialize the Hall Sensor device
        \param config Configuration settings
    */
    void Init(Config config)
    {
        config_ = config;

        TIM_HallSensor_InitTypeDef hall_conf_;

        hall_conf_.IC1Polarity       = config_.polarity;
        hall_conf_.IC1Prescaler      = config_.prescaler;
        hall_conf_.IC1Filter         = config_.filter;
        hall_conf_.Commutation_Delay = config_.commutation_delay;

        HAL_TIMEx_HallSensor_Init(&hall_, &hall_conf_);
    }

    void StartBlockingRead()
    {
        // are we ready, kids?
        while(HAL_TIMEx_HallSensor_GetState(&hall_) != HAL_TIM_STATE_READY) {};

        HAL_TIMEx_HallSensor_Start(&hall_);
    }

    uint8_t GetCount() { return hall_.Instance->CNT; }

    void InitPins()
    {
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_TypeDef*    port;

        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT; // is this right?
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
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

        port                = dsy_hal_map_get_port(&config_.pin_);
        GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&config_.pin_);
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    }

  private:
    Config            config_;
    TIM_HandleTypeDef hall_;

    /** @} */

}; // Class

extern "C"
{
    void PinInit() {}

    void HAL_TIMEx_HallSensor_MspInit(TIM_HandleTypeDef* htim)
    {
        PinInit();
        if(htim->Instance == TIM1)
        {
            // __HAL_RCC_GPIOx_CLK_ENABLE(); // which ports go with which timers?
            __HAL_RCC_TIM1_CLK_ENABLE();
        }
        if(htim->Instance == TIM2)
        {
            // __HAL_RCC_GPIOx_CLK_ENABLE();
            __HAL_RCC_TIM2_CLK_ENABLE();
        }
        else if(htim->Instance == TIM3)
        {
            // __HAL_RCC_GPIOx_CLK_ENABLE();
            __HAL_RCC_TIM3_CLK_ENABLE();
        }
        else if(htim->Instance == TIM4)
        {
            // __HAL_RCC_GPIOx_CLK_ENABLE();
            __HAL_RCC_TIM4_CLK_ENABLE();
        }
        else if(htim->Instance == TIM5)
        {
            // __HAL_RCC_GPIOx_CLK_ENABLE();
            __HAL_RCC_TIM5_CLK_ENABLE();
        }
        else if(htim->Instance == TIM6)
        {
            // __HAL_RCC_GPIOx_CLK_ENABLE();
            __HAL_RCC_TIM6_CLK_ENABLE();
        }
    }
} // extern C

} // namespace daisy
#endif
