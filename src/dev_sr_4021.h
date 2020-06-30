#pragma once
#ifndef DEV_SR_4021_H
/** & */
#define DEV_SR_4021_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "per_gpio.h"

/** Fixed maximums for parallel/daisychained use \n 
    These could be expanded, but haven't been tested beyond this
*/
#define SR_4021_MAX_PARALLEL 2
    /** fixed maximum for daisychained use */
#define SR_4021_MAX_DAISYCHAIN 1


    /** 
    @brief Device driver for the CD4021. 
    Bit-banged serial shift input.
    */

    /** @addtogroup shiftregister
    @{
    */


    /** Pins that need to be configured to use. 
      DATA2 only needs to be set if num_parallel is > 1
  */
    enum
    {
        DSY_SR_4021_PIN_CS,    /**< CS Pin */
        DSY_SR_4021_PIN_CLK,   /**< CLK Pin */
        DSY_SR_4021_PIN_DATA,  /**< DATA pin */
        DSY_SR_4021_PIN_DATA2, /**< DATA2 Pin, optional */
        DSY_SR_4021_PIN_LAST,  /**< Enum Last */
    };


    /** configuration strucutre for 4021
      pin config is used to initialize the dsy_gpio
      num_parallel is the number of devices connected 
      that share the same clk/cs, etc. but have independent data
      num_daisychained is the number of devices in a daisy-chain configuration
*/
    typedef struct
    {
        dsy_gpio_pin pin_config
            [DSY_SR_4021_PIN_LAST];     /**< used to initialize the dsy_gpio */
        uint8_t num_parallel,           /**< number of devices connected */
            num_daisychained;           /**< Number of devices daisy chained */
        dsy_gpio cs,                    /**< cs pin */
            clk,                        /**< clk pin */
            data[SR_4021_MAX_PARALLEL]; /**< array of data pins */
        uint8_t states[8 * SR_4021_MAX_DAISYCHAIN
                       * SR_4021_MAX_PARALLEL]; /**< array of states */
    } dsy_sr_4021_handle;

    /** Initialize CD4021 with settings from sr_4021_handle 
    \param sr handle to initialize 
    */
    void dsy_sr_4021_init(dsy_sr_4021_handle *sr);

    /** Fills internal states with CD4021 data states.
    \param *sr Handle to update
     */
    void dsy_sr_4021_update(
        dsy_sr_4021_handle *sr); // checks all 8 states per configured device.

    /** Returns the state of a pin at a given index.
    \param *sr Handle containing desired pin
    \param idx Pin index
    */
    uint8_t dsy_sr_4021_state(dsy_sr_4021_handle *sr, uint8_t idx);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
