#include "stm32h7xx_hal.h"
#include "per/gpio.h"
#include "util/hal_map.h"

static void start_clock_for_pin(const dsy_gpio *p)
{
    switch(p->pin.port)
    {
        case DSY_GPIOA: __HAL_RCC_GPIOA_CLK_ENABLE(); break;
        case DSY_GPIOB: __HAL_RCC_GPIOB_CLK_ENABLE(); break;
        case DSY_GPIOC: __HAL_RCC_GPIOC_CLK_ENABLE(); break;
        case DSY_GPIOD: __HAL_RCC_GPIOD_CLK_ENABLE(); break;
        case DSY_GPIOE: __HAL_RCC_GPIOE_CLK_ENABLE(); break;
        case DSY_GPIOF: __HAL_RCC_GPIOF_CLK_ENABLE(); break;
        case DSY_GPIOG: __HAL_RCC_GPIOG_CLK_ENABLE(); break;
        case DSY_GPIOH: __HAL_RCC_GPIOH_CLK_ENABLE(); break;
        case DSY_GPIOI: __HAL_RCC_GPIOI_CLK_ENABLE(); break;
        default: break;
    }
}

void dsy_gpio_init(const dsy_gpio *p)
{
    GPIO_TypeDef *   port;
    GPIO_InitTypeDef ginit;
    switch(p->mode)
    {
        case DSY_GPIO_MODE_INPUT: ginit.Mode = GPIO_MODE_INPUT; break;
        case DSY_GPIO_MODE_OUTPUT_PP: ginit.Mode = GPIO_MODE_OUTPUT_PP; break;
        case DSY_GPIO_MODE_OUTPUT_OD: ginit.Mode = GPIO_MODE_OUTPUT_OD; break;
        case DSY_GPIO_MODE_ANALOG: ginit.Mode = GPIO_MODE_ANALOG; break;
        default: ginit.Mode = GPIO_MODE_INPUT; break;
    }
    switch(p->pull)
    {
        case DSY_GPIO_NOPULL: ginit.Pull = GPIO_NOPULL; break;
        case DSY_GPIO_PULLUP: ginit.Pull = GPIO_PULLUP; break;
        case DSY_GPIO_PULLDOWN: ginit.Pull = GPIO_PULLDOWN; break;
        default: ginit.Pull = GPIO_NOPULL; break;
    }
    ginit.Speed = GPIO_SPEED_LOW;
    port        = dsy_hal_map_get_port(&p->pin);
    ginit.Pin   = dsy_hal_map_get_pin(&p->pin);
    start_clock_for_pin(p);
    HAL_GPIO_Init(port, &ginit);
}

void dsy_gpio_deinit(const dsy_gpio *p)
{
    GPIO_TypeDef *port;
    uint16_t      pin;
    port = dsy_hal_map_get_port(&p->pin);
    pin  = dsy_hal_map_get_pin(&p->pin);
    HAL_GPIO_DeInit(port, pin);
}

uint8_t dsy_gpio_read(const dsy_gpio *p)
{
    return HAL_GPIO_ReadPin(dsy_hal_map_get_port(&p->pin),
                            dsy_hal_map_get_pin(&p->pin));
    //    return HAL_GPIO_ReadPin((GPIO_TypeDef *)gpio_hal_port_map[p->pin.port],
    //                            gpio_hal_pin_map[p->pin.pin]);
}

void dsy_gpio_write(const dsy_gpio *p, uint8_t state)
{
    return HAL_GPIO_WritePin(dsy_hal_map_get_port(&p->pin),
                             dsy_hal_map_get_pin(&p->pin),
                             (GPIO_PinState)(state > 0 ? 1 : 0));
    //    HAL_GPIO_WritePin((GPIO_TypeDef *)gpio_hal_port_map[p->pin.port],
    //                      gpio_hal_pin_map[p->pin.pin],
    //                      (GPIO_PinState)(state > 0 ? 1 : 0));
}
void dsy_gpio_toggle(const dsy_gpio *p)
{
    return HAL_GPIO_TogglePin(dsy_hal_map_get_port(&p->pin),
                              dsy_hal_map_get_pin(&p->pin));
    //    HAL_GPIO_TogglePin((GPIO_TypeDef *)gpio_hal_port_map[p->pin.port],
    //                       gpio_hal_pin_map[p->pin.pin]);
}
