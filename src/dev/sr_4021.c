#include "dev/sr_4021.h"
#include "per/tim.h"

void dsy_sr_4021_init(dsy_sr_4021_handle *sr)
{
    sr->cs.mode     = DSY_GPIO_MODE_OUTPUT_PP;
    sr->cs.pull     = DSY_GPIO_NOPULL;
    sr->cs.pin.port = sr->pin_config[DSY_SR_4021_PIN_CS].port;
    sr->cs.pin.pin  = sr->pin_config[DSY_SR_4021_PIN_CS].pin;
    dsy_gpio_init(&sr->cs);

    sr->clk.mode     = DSY_GPIO_MODE_OUTPUT_PP;
    sr->clk.pull     = DSY_GPIO_NOPULL;
    sr->clk.pin.port = sr->pin_config[DSY_SR_4021_PIN_CLK].port;
    sr->clk.pin.pin  = sr->pin_config[DSY_SR_4021_PIN_CLK].pin;
    dsy_gpio_init(&sr->clk);

    sr->data[0].mode     = DSY_GPIO_MODE_INPUT;
    sr->data[0].pull     = DSY_GPIO_NOPULL;
    sr->data[0].pin.port = sr->pin_config[DSY_SR_4021_PIN_DATA].port;
    sr->data[0].pin.pin  = sr->pin_config[DSY_SR_4021_PIN_DATA].pin;
    dsy_gpio_init(&sr->data[0]);
    if(sr->num_parallel > 1)
    {
        sr->data[1].mode     = DSY_GPIO_MODE_INPUT;
        sr->data[1].pull     = DSY_GPIO_NOPULL;
        sr->data[1].pin.port = sr->pin_config[DSY_SR_4021_PIN_DATA2].port;
        sr->data[1].pin.pin  = sr->pin_config[DSY_SR_4021_PIN_DATA2].pin;
        dsy_gpio_init(&sr->data[1]);
    }
}

// Data read in so that daisy chain device states are sequential,
// parallel data is offset by 8*num_daisychain
// takes about 18-20us * num_daisychained
//void __attribute__((optimize("O0"))) dsy_sr_4021_update(dsy_sr_4021_handle *sr)

// 3000 = 15us max clock/rise/fall time
void dsy_sr_4021_update(dsy_sr_4021_handle *sr)
{
    uint8_t idx;
    // Strobe CS Pin
    dsy_gpio_write(&sr->clk, 0);
    dsy_gpio_write(&sr->cs, 1);
    dsy_tim_delay_tick(1);
    dsy_gpio_write(&sr->cs, 0);
    // Clock through data.
    for(uint8_t i = 0; i < 8 * sr->num_daisychained; i++)
    {
        // Grab data from each parallel data pin.
        dsy_gpio_write(&sr->clk, 0);
        dsy_tim_delay_tick(1);
        for(uint8_t j = 0; j < sr->num_parallel; j++)
        {
            idx = (8 * sr->num_daisychained - 1) - i;
            idx += (8 * sr->num_daisychained * j);
            sr->states[idx] = dsy_gpio_read(&sr->data[j]);
        }
        dsy_gpio_write(&sr->clk, 1);
        dsy_tim_delay_tick(1);
    }
}

uint8_t dsy_sr_4021_state(dsy_sr_4021_handle *sr, uint8_t idx)
{
    return sr
        ->states[idx < 8 * sr->num_daisychained * sr->num_parallel ? idx : 0];
}
