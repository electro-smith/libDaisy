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

    void Init(Pin pina, Pin pinb)
    {
        GPIO::Config gpio_conf;
        gpio_conf.pin  = pina;
        gpio_conf.pull = GPIO::Config::Pull::PULLUP;
        pina_gpio_.Init(gpio_conf);

        gpio_conf.pin = pinb;
        pinb_gpio_.Init(gpio_conf);
    }

    int Read()
    {
        if(!pina_gpio_.Read())
            return POS_UP;
        if(!pinb_gpio_.Read())
            return POS_DOWN;
        return POS_CENTER;
    }

  private:
    GPIO pina_gpio_;
    GPIO pinb_gpio_;
};

} // namespace daisy
