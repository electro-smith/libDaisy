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
    void InitSingle(dsy_gpio_pin pin);
    // ~~~~

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
                 size_t       channels);
    // ~~~~

    dsy_gpio pin_;
    dsy_gpio mux_pin_[MUX_SEL_LAST];
    uint8_t  mux_channels_;
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
	// * ovs: Oversampling amount - Defaults to OVS_32
    // ~~~~
    void Init(AdcChannelConfig *cfg, size_t num_channels, OverSampling ovs=OVS_32);
    // ~~~~
    //
    // ### Start
	// Starts reading from the ADC
	// ~~~~
    void Start();
	// ~~~~

    // ### Stop
	// Stops reading from the ADC
	// ~~~~
    void Stop();
	// ~~~~

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
    OverSampling oversampling_;
    size_t       num_channels_;
};

} // namespace daisy

#endif // DSY_ADC_H
