#include "per/tim.h"
#include <stm32h7xx_hal.h>

enum
{
    SCALE_MS,
    SCALE_US,
    SCALE_NS,
    SCALE_LAST,
};

typedef struct
{
    uint32_t          scale[SCALE_LAST];
    TIM_HandleTypeDef htim2;
} dsy_tim;

static void sthal_tim_init();

static dsy_tim tim;

void dsy_tim_init()
{
    tim.scale[SCALE_MS] = 200000;
    tim.scale[SCALE_US] = 200;
    tim.scale[SCALE_NS] = 2;
    sthal_tim_init();
}
void dsy_tim_start()
{
    HAL_TIM_Base_Start(&tim.htim2);
}

uint32_t dsy_tim_get_tick()
{
    return tim.htim2.Instance->CNT;
}

void dsy_tim_delay_tick(uint32_t cnt)
{
    uint32_t now;
    now = dsy_tim_get_tick();
    while(dsy_tim_get_tick() - now < cnt) {}
}
uint32_t dsy_tim_get_ms()
{
    return tim.htim2.Instance->CNT / tim.scale[SCALE_MS];
}
void dsy_tim_delay_ms(uint32_t cnt)
{
    dsy_tim_delay_tick(cnt * tim.scale[SCALE_MS]);
}
uint32_t dsy_tim_get_us()
{
    return tim.htim2.Instance->CNT / tim.scale[SCALE_US];
}

void dsy_tim_delay_us(uint32_t cnt)
{
    dsy_tim_delay_tick(cnt * tim.scale[SCALE_US]);
}

// STM32 HAL Stuff below

static void sthal_tim_init()
{
    TIM_ClockConfigTypeDef  sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig      = {0};

    tim.htim2.Instance           = TIM2;
    tim.htim2.Init.Prescaler     = 0;
    tim.htim2.Init.CounterMode   = TIM_COUNTERMODE_UP;
    tim.htim2.Init.Period        = 0xffffffff;
    tim.htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    //tim.htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    tim.htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if(HAL_TIM_Base_Init(&tim.htim2) != HAL_OK)
    {
        //    Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if(HAL_TIM_ConfigClockSource(&tim.htim2, &sClockSourceConfig) != HAL_OK)
    {
        //    Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    if(HAL_TIMEx_MasterConfigSynchronization(&tim.htim2, &sMasterConfig)
       != HAL_OK)
    {
        //    Error_Handler();
    }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{
    if(tim_baseHandle->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{
    if(tim_baseHandle->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_DISABLE();
    }
}
