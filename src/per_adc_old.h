#pragma once
#ifndef DSY_ADC_H
#define DSY_ADC_H
#include <stdint.h>
#include <stdlib.h>
#include "daisy_core.h"

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

    // ### InitSingle
    // Initializes a single ADC pin as an ADC.
    // ~~~~
    void InitSingle(dsy_gpio_pin *pin)
    // ~~~~
    {
        pin_config_ = *pin;
        mux_channels_ = 0;
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
    void InitMux(dsy_gpio_pin *adc_pin, 
            dsy_gpio_pin *mux_0, 
            dsy_gpio_pin *mux_1, 
            dsy_gpio_pin *mux_2,
            size_t channels)
    // ~~~~
    {
        size_t pins_to_init;
        pin_config_ = *adc_pin;
        mux_pin_[0].pin = *mux_0;
        mux_pin_[1].pin = *mux_1;
        mux_pin_[2].pin = *mux_2;
        num_channels_ = channels <  8 ? channels : 8;
        pins_to_init = (num_channels_ - 1) >> 1;
        for (size_t i = 0; i <= pins_to_init; i++)
        {
            dsy_gpio_init(&mux_pin[i]);
            mux_pin_[i].mode = DSY_GPIO_MODE_OUTPUT_PP;
            mux_pin_[i].pull = DSY_GPIO_NOPULL;
        }
    }

    enum MuxPin
    {
        MUX_SEL_0,
        MUX_SEL_1,
        MUX_SEL_2,
        MUX_SEL_LAST,
    };

    dsy_gpio_pin         pin_config_;
    dsy_gpio             mux_pin_[MUX_SEL_LAST];
    uint8_t              mux_channels_;
}

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
    void Init(AdcChannelConfig *cfg, size_t num_channels);
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
    const uint8_t kMaxChannels = 13;
    AdcChannelConfig config_[kMaxChannels];
    Oversampling     oversampling_;
};

} // namespace daisy

#ifdef __cplusplus
extern "C"
{
#endif

    // Limitations:
    // - For now speed is fixed at ASYNC_DIV128 for ADC Clock, and SAMPLETIME_64CYCLES_5 for each conversion.
    // - No OPAMP config for the weird channel
    // - No oversampling built in yet

    typedef enum
    {
        DSY_ADC_PIN_CHN3,
        DSY_ADC_PIN_CHN4,
        DSY_ADC_PIN_CHN5,
        DSY_ADC_PIN_CHN7,
        DSY_ADC_PIN_CHN8,
        DSY_ADC_PIN_CHN9,
        DSY_ADC_PIN_CHN10,
        DSY_ADC_PIN_CHN11,
        DSY_ADC_PIN_CHN14,
        DSY_ADC_PIN_CHN15,
        DSY_ADC_PIN_CHN16,
        DSY_ADC_PIN_CHN17,
        DSY_ADC_PIN_CHN18,
        DSY_ADC_PIN_CHN19,
        DSY_ADC_PIN_LAST,
    } dsy_adc_pin;

    typedef enum
    {
        MUX_SEL_0,
        MUX_SEL_1,
        MUX_SEL_2,
        MUX_SEL_LAST,
    } dsy_adc_mux_pin;

    typedef enum
    {
        DSY_ADC_OVS_NONE,
        DSY_ADC_OVS_4,
        DSY_ADC_OVS_8,
        DSY_ADC_OVS_16,
        DSY_ADC_OVS_32,
        DSY_ADC_OVS_64,
        DSY_ADC_OVS_128,
        DSY_ADC_OVS_256,
        DSY_ADC_OVS_512,
        DSY_ADC_OVS_1024,
        DSY_ADC_OVS_LAST,
    } dsy_adc_oversampling;

    // Old style
    typedef struct
    {
        dsy_gpio_pin         pin_config[DSY_ADC_PIN_LAST];
        dsy_gpio_pin         mux_pin_config[DSY_ADC_PIN_LAST][MUX_SEL_LAST];
        dsy_adc_oversampling oversampling;
        uint8_t              active_channels[DSY_ADC_PIN_LAST];
        uint8_t              channels, mux_channels[DSY_ADC_PIN_LAST];
    } dsy_adc_handle;

    void      dsy_adc_init(dsy_adc_handle *dsy_hadc);
    void      dsy_adc_start();
    void      dsy_adc_stop();
    uint16_t  dsy_adc_get(uint8_t chn);
    uint16_t *dsy_adc_get_rawptr(uint8_t chn);
    float     dsy_adc_get_float(uint8_t chn);

    // These are getters for multiplexed inputs on a single channel (up to 8 per ADC input).
    uint16_t  dsy_adc_get_mux(uint8_t chn, uint8_t idx);
    uint16_t *dsy_adc_get_mux_rawptr(uint8_t chn, uint8_t idx);
    float     dsy_adc_get_mux_float(uint8_t chn, uint8_t idx);

#ifdef __cplusplus
}
#endif
#endif // DSY_ADC_H
