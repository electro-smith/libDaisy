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
        pina_gpio_.Init(pina, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
        pinb_gpio_.Init(pinb, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
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
