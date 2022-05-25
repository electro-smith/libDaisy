#pragma once
#ifndef DSY_ADC_H
#define DSY_ADC_H /**< Macro */
#include <stdint.h>
#include <stdlib.h>
#include "daisy_core.h"
#include "per/gpio.h"

#define DSY_ADC_MAX_CHANNELS 16 /**< Maximum number of ADC channels */

namespace daisy
{
/** @addtogroup per_analog
    @{
*/

/* While there may not be many configuration options here,
using a struct like this allows us to add more configuration
later without breaking existing functionality.
*/

/** @brief   Configuration Structure for an ADC Channel 
 *  @details This can be used to configure a a single input, 
 *           or a multiplexed input, allowing up to 8 inputs on 
 *           one channel.
 *  @note    Sharing data lines to multiple muxes _is_ possible, but
 *           each channel sharing data lines must be set to the maximum
 *           number of channels, even if some multiplexers have fewer
 *           inputs connected.
*/
struct AdcChannelConfig
{
    /** Which pin to use for multiplexing */
    enum MuxPin
    {
        MUX_SEL_0,    /**< & */
        MUX_SEL_1,    /**< & */
        MUX_SEL_2,    /**< & */
        MUX_SEL_LAST, /**< & */
    };

    /** Initializes a single ADC pin as an ADC. 
    \param pin Pin to init.
     */
    void InitSingle(dsy_gpio_pin pin);

    /** 
    Initializes a single ADC pin as a Multiplexed ADC.
    Requires a CD405X Multiplexer connected to the pin.
    You only need to supply the mux pins that are required,
    e.g. a 4052 mux would only require mux_0 and mux_1.
    Internal Callbacks handle the pin addressing.
    \param mux_channels must be 1-8
    \param mux_0 First mux pin
    \param mux_1 Second mux pin
    \param mux_2 Third mux pin
    \param adc_pin &
    */
    void InitMux(dsy_gpio_pin adc_pin,
                 size_t       mux_channels,
                 dsy_gpio_pin mux_0,
                 dsy_gpio_pin mux_1 = {DSY_GPIOX, 0},
                 dsy_gpio_pin mux_2 = {DSY_GPIOX, 0});

    dsy_gpio pin_;                   /**< & */
    dsy_gpio mux_pin_[MUX_SEL_LAST]; /**< & */
    uint8_t  mux_channels_;          /**< & */
};

/**
   @brief Handler for analog to digital conversion
*/
class AdcHandle
{
  public:
    /** Supported oversampling amounts */
    enum OverSampling
    {
        OVS_NONE, /**< & */
        OVS_4,    /**< & */
        OVS_8,    /**< & */
        OVS_16,   /**< & */
        OVS_32,   /**< & */
        OVS_64,   /**< & */
        OVS_128,  /**< & */
        OVS_256,  /**< & */
        OVS_512,  /**< & */
        OVS_1024, /**< & */
        OVS_LAST, /**< & */
    };

    AdcHandle() {}
    ~AdcHandle() {}
    /** 
    Initializes the ADC with the pins passed in.
    \param *cfg an array of AdcChannelConfig of the desired channel
    \param num_channels number of ADC channels to initialize
    \param ovs Oversampling amount - Defaults to OVS_32
    */
    void
    Init(AdcChannelConfig *cfg, size_t num_channels, OverSampling ovs = OVS_32);

    /** Starts reading from the ADC */
    void Start();

    /** Stops reading from the ADC */
    void Stop();

    /** 
    Single channel getter
    \param chn channel to get
    \return Converted value
    */
    uint16_t Get(uint8_t chn) const;

    /**
       Get pointer to a value from a single channel
       \param chn
       \return Pointer to converted value
    */
    uint16_t *GetPtr(uint8_t chn) const;

    /** 
    Get floating point from single channel
    \param chn Channel to get from
    \return Floating point converted value
    */
    float GetFloat(uint8_t chn) const;

    /**
       Getters for multiplexed inputs on a single channel (up to 8 per ADC input). 
       \param chn Channel to get from
       \param idx &
       \return data
    */
    uint16_t GetMux(uint8_t chn, uint8_t idx) const;

    /**
       Getters for multiplexed inputs on a single channel. (Max 8 per chan)
       \param chn Channel to get from
       \param idx &
       \return Pointer to data
    */
    uint16_t *GetMuxPtr(uint8_t chn, uint8_t idx) const;

    /**
       Getters for multiplexed inputs on a single channel (up to 8 per ADC input). 
       \param chn Channel to get from
       \param idx &
       \return Floating point data
    */
    float GetMuxFloat(uint8_t chn, uint8_t idx) const;

  private:
    OverSampling oversampling_;
    size_t       num_channels_;
};

/** @} */
} // namespace daisy

#endif // DSY_ADC_H
