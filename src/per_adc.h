#pragma once
#ifndef DSY_ADC_H
#define DSY_ADC_H
#include <stdint.h>
#include <stdlib.h>
#include "daisy_core.h"
#include "per_gpio.h"

#define DSY_ADC_MAX_CHANNELS 14

namespace daisy
{

// ## AdcChannelConfig
// Configuration Structure for a given channel 
// 
// While there may not be many configuration options here,
// using a struct like this allows us to add more configuration
// later without breaking existing functionality.
// ~~~~
struct AdcChannelConfig
// ~~~~
{
    enum MuxPin
    {
        MUX_SEL_0,
        MUX_SEL_1,
        MUX_SEL_2,
        MUX_SEL_LAST,
    };

    // ### InitSingle
    // Initializes a single ADC pin as an ADC.
    // ~~~~
    void InitSingle(dsy_gpio_pin pin)
    // ~~~~
    {
        pin_.pin = pin;
        mux_channels_ = 0;
        pin_.mode = DSY_GPIO_MODE_ANALOG;
        pin_.pull = DSY_GPIO_NOPULL;
        dsy_gpio_init(&pin_);
    }

    // ### InitMux
    // Initializes a single ADC pin as a Multiplexed ADC.
    //
    // Requires a CD4051 Multiplexor connected to the pin
    //
    // Internal Callbacks handle the pin addressing.
    // 
    // channels must be 1-8
    // ~~~~
    void InitMux(dsy_gpio_pin adc_pin, 
            dsy_gpio_pin mux_0, 
            dsy_gpio_pin mux_1, 
            dsy_gpio_pin mux_2,
            size_t channels)
    // ~~~~
    {
        size_t pins_to_init;
        // Init ADC Pin
        pin_.pin = adc_pin;
        pin_.mode = DSY_GPIO_MODE_ANALOG;
        pin_.pull = DSY_GPIO_NOPULL;
        dsy_gpio_init(&pin_);
        // Init Muxes
        mux_pin_[0].pin = mux_0;
        mux_pin_[1].pin = mux_1;
        mux_pin_[2].pin = mux_2;
        mux_channels_ = channels <  8 ? channels : 8;
        pins_to_init = (mux_channels_ - 1) >> 1;
        for (size_t i = 0; i <= pins_to_init; i++)
        {
            dsy_gpio_init(&mux_pin_[i]);
            mux_pin_[i].mode = DSY_GPIO_MODE_OUTPUT_PP;
            mux_pin_[i].pull = DSY_GPIO_NOPULL;
        }
    }


    dsy_gpio             pin_;
    dsy_gpio             mux_pin_[MUX_SEL_LAST];
    uint8_t              mux_channels_;
};

class AdcHandle
{
  public:

    enum OverSampling
    {
        OVS_NONE,
        OVS_4,
        OVS_8,
        OVS_16,
        OVS_32,
        OVS_64,
        OVS_128,
        OVS_256,
        OVS_512,
        OVS_1024,
        OVS_LAST,
    };

    AdcHandle() {}
    ~AdcHandle() {}
    // ### Init
    // Initializes the ADC with the pins passed in.
    // 
    // * *cfg: an array of AdcChannelConfig of the desired channel
    // * num_channels: number of ADC channels to initialize
    // ~~~~
    void Init(AdcChannelConfig *cfg, size_t num_channels, OverSampling ovs);
    // ~~~~
    // 
    // ### Start
    void Start();

    // ### Stop
    void      Stop();

    // ## Accessors
    //
    // These are getters for a single channel
    // ~~~~
    uint16_t  Get(uint8_t chn);
    uint16_t *GetPtr(uint8_t chn);
    float     GetFloat(uint8_t chn);
    // ~~~~
    // These are getters for multiplexed inputs on a single channel (up to 8 per ADC input).
    // ~~~~
    uint16_t  GetMux(uint8_t chn, uint8_t idx);
    uint16_t *GetMuxPtr(uint8_t chn, uint8_t idx);
    float     GetMuxFloat(uint8_t chn, uint8_t idx);
    // ~~~~



  private:
    OverSampling     oversampling_;
    AdcChannelConfig config_[DSY_ADC_MAX_CHANNELS];
    size_t num_channels_;
};

} // namespace daisy

#endif // DSY_ADC_H
