#pragma once
#include "daisy_core.h"
#include "per/gpio.h"

namespace daisy
{
class Switch3
{
  public:
    enum
    {
        POS_CENTER = 0,
        POS_LEFT   = 1,
        POS_UP     = 1,
        POS_RIGHT  = 2,
        POS_DOWN   = 2,
    };


    Switch3() {}
    ~Switch3() {}

    void Init(dsy_gpio_pin pina, dsy_gpio_pin pinb)
    {
        pina_gpio_.pin  = pina;
        pina_gpio_.mode = DSY_GPIO_MODE_INPUT;
        pina_gpio_.pull = DSY_GPIO_PULLUP;
        dsy_gpio_init(&pina_gpio_);

        pinb_gpio_.pin  = pinb;
        pinb_gpio_.mode = DSY_GPIO_MODE_INPUT;
        pinb_gpio_.pull = DSY_GPIO_PULLUP;
        dsy_gpio_init(&pinb_gpio_);
    }

    int Read()
    {
        if(!dsy_gpio_read(&pina_gpio_))
            return POS_UP;
        if(!dsy_gpio_read(&pinb_gpio_))
            return POS_DOWN;
        return POS_CENTER;
    }

  private:
    dsy_gpio pina_gpio_;
    dsy_gpio pinb_gpio_;
};

} // namespace daisy
