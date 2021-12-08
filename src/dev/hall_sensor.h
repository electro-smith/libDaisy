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

        dsy_gpio_pin pin;

        Config()
        {
            periph = Peripheral::TIM_3;
            pin    = {DSY_GPIOA, 8};

            polarity
                = TIM_ICPOLARITY_RISING; // options are rising, falling, bothedge
            prescaler = TIM_ICPSC_DIV1;  // options are 1, 2, 4, 8
            filter    = 0x05; // range is 0x0 -> 0xF basically the debounce
            commutation_delay = 0x0000; // range is 0x0000 -> 0xFFFF
        }
    };

    /** Initialize the Hall Sensor device
        \param config Configuration settings
    */
    void Init(Config config);

    /** Start reading in a blocking fashion. Init calls this by default. */
    void StartBlockingRead();

    /** Stop reading in a blocking fashion */
    void StopBlockingRead() { HAL_TIMEx_HallSensor_Stop(&hall_); }

    /** Get the total count of hall sensor clicks so far 
        \return Total number of times hall sensor has gone high since init.
    */
    uint8_t GetCount() { return hall_.Instance->CNT; }

  private:
    Config            config_;
    TIM_HandleTypeDef hall_;

    /** Get the hall sensor pin going and working with the correct clock */
    void InitPins()
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

        port                = dsy_hal_map_get_port(&config_.pin);
        GPIO_InitStruct.Pin = dsy_hal_map_get_pin(&config_.pin);
        HAL_GPIO_Init(port, &GPIO_InitStruct);
        dsy_hal_map_gpio_clk_enable(config_.pin.port);
    }

    /** Deinit the hall sensor pin. Unused as of now. */
    void DeInitPins()
    {
        GPIO_TypeDef* port = dsy_hal_map_get_port(&config_.pin);
        uint16_t      pin  = dsy_hal_map_get_pin(&config_.pin);
        HAL_GPIO_DeInit(port, pin);
    }

    /** @} */
}; // HallSensor

} // namespace daisy
#endif
