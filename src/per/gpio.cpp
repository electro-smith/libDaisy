#include "gpio.h"
#include "stm32h7xx_hal.h"

using namespace daisy;

void GPIO::Init(const Config &cfg)
{
    /** Copy Config */
    cfg_ = cfg;

    if(!cfg_.pin.IsValid())
        return;

    GPIO_InitTypeDef ginit;
    switch(cfg_.mode)
    {
        case Mode::OUTPUT: ginit.Mode = GPIO_MODE_OUTPUT_PP; break;
        case Mode::OUTPUT_OD: ginit.Mode = GPIO_MODE_OUTPUT_OD; break;
        case Mode::ANALOG: ginit.Mode = GPIO_MODE_ANALOG; break;
        case Mode::INPUT:
        default: ginit.Mode = GPIO_MODE_INPUT; break;
    }
    switch(cfg_.pull)
    {
        case Pull::PULLUP: ginit.Pull = GPIO_PULLUP; break;
        case Pull::PULLDOWN: ginit.Pull = GPIO_PULLDOWN; break;
        case Pull::NOPULL:
        default: ginit.Pull = GPIO_NOPULL;
    }
    switch(cfg_.speed)
    {
        case Speed::VERY_HIGH: ginit.Speed = GPIO_SPEED_FREQ_VERY_HIGH; break;
        case Speed::HIGH: ginit.Speed = GPIO_SPEED_FREQ_HIGH; break;
        case Speed::MEDIUM: ginit.Speed = GPIO_SPEED_FREQ_MEDIUM; break;
        case Speed::LOW:
        default: ginit.Speed = GPIO_SPEED_FREQ_LOW;
    }

    port_base_addr_ = GetGPIOBaseRegister();
    /** Start Relevant GPIO clock */
    switch(cfg_.pin.port)
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
    /** Set pin based on stm32 schema */
    ginit.Pin = (1 << cfg_.pin.pin);
    HAL_GPIO_Init((GPIO_TypeDef *)port_base_addr_, &ginit);
}
void GPIO::Init(Pin p, const Config &cfg)
{
    /** Copy config */
    cfg_ = cfg;
    /** Overwrite with explicit pin */
    cfg_.pin = p;
    Init(cfg_);
}
void GPIO::Init(Pin p, Mode m, Pull pu, Speed sp)
{
    // Populate Config struct, and init with overload
    cfg_.pin   = p;
    cfg_.mode  = m;
    cfg_.pull  = pu;
    cfg_.speed = sp;
    Init(cfg_);
}

void GPIO::DeInit()
{
    if(cfg_.pin.IsValid())
        HAL_GPIO_DeInit((GPIO_TypeDef *)port_base_addr_, (1 << cfg_.pin.pin));
}
bool GPIO::Read()
{
    return HAL_GPIO_ReadPin((GPIO_TypeDef *)port_base_addr_,
                            (1 << cfg_.pin.pin));
}
void GPIO::Write(bool state)
{
    HAL_GPIO_WritePin((GPIO_TypeDef *)port_base_addr_,
                      (1 << cfg_.pin.pin),
                      (GPIO_PinState)state);
}
void GPIO::Toggle()
{
    HAL_GPIO_TogglePin((GPIO_TypeDef *)port_base_addr_, (1 << cfg_.pin.pin));
}

uint32_t *GPIO::GetGPIOBaseRegister()
{
    switch(cfg_.pin.port)
    {
        case PORTA: return (uint32_t *)GPIOA;
        case PORTB: return (uint32_t *)GPIOB;
        case PORTC: return (uint32_t *)GPIOC;
        case PORTD: return (uint32_t *)GPIOD;
        case PORTE: return (uint32_t *)GPIOE;
        case PORTF: return (uint32_t *)GPIOF;
        case PORTG: return (uint32_t *)GPIOG;
        case PORTH: return (uint32_t *)GPIOH;
        case PORTI: return (uint32_t *)GPIOI;
        case PORTJ: return (uint32_t *)GPIOJ;
        case PORTK: return (uint32_t *)GPIOK;
        default: return NULL;
    }
}

// #include "stm32h7xx_hal.h"
// #include "per/gpio.h"

/** OLD C Stuff */
extern "C"
{
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
            case DSY_GPIO_MODE_OUTPUT_PP:
                ginit.Mode = GPIO_MODE_OUTPUT_PP;
                break;
            case DSY_GPIO_MODE_OUTPUT_OD:
                ginit.Mode = GPIO_MODE_OUTPUT_OD;
                break;
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
}
