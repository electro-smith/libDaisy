#include "dev_codec_ak4556.h"
#include "sys_system.h"
#include "per_gpio.h"

void codec_ak4556_init(dsy_gpio_pin reset_pin)
{
    dsy_gpio reset;
    reset.pin = reset_pin;
    reset.mode = DSY_GPIO_MODE_OUTPUT_PP;
    reset.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&reset);
    dsy_gpio_write(&reset, 0);
    dsy_system_delay(1); // Datasheet specifies minimum 150ns
    dsy_gpio_write(&reset, 1);
}
