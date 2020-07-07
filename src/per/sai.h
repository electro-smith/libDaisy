/* 
TODO   
- Fix hard-coding of SAI2 Initialization
- Clean up this and hid_audio (the lines between them are a bit blurry).
- Variable Samplerate setting (should be fine with the new clock tree)
- Fix intermingling of both SAI. they should be able to be independently initialized (like i2c, etc.)
- Add 32-bit bitdepth (for devices that may need that)
- Move the 'device' chunk of this to hid_audio. given that they sometimes need other pins or i2c, etc.
*/
#ifndef DSY_SAI_H
#define DSY_SAI_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "daisy_core.h"

    /** @addtogroup serial
    @{
    */

    /** Driver for the SAI peripheral Supports SAI1 and SAI2 with several configuration options */

    /** selects which SAI (or both/none) to initialize */
    typedef enum
    {
        DSY_AUDIO_INIT_SAI1, /**< & */
        DSY_AUDIO_INIT_SAI2, /**< & */
        DSY_AUDIO_INIT_BOTH, /**< & */
        DSY_AUDIO_INIT_NONE, /**< & */
        DSY_AUDIO_INIT_LAST, /**< & */
    } dsy_audio_sai;

    /**  Currently Sample Rates are not correctly supported. All audio is currently run at 48kHz */
    typedef enum
    {
        DSY_AUDIO_SAMPLERATE_32K,  /**< & */
        DSY_AUDIO_SAMPLERATE_48K,  /**< & */
        DSY_AUDIO_SAMPLERATE_96K,  /**< & */
        DSY_AUDIO_SAMPLERATE_LAST, /**< & */
    } dsy_audio_samplerate;

    /** Specifies the bitdepth of the hardware connected to the SAI peripheral */
    typedef enum
    {
        DSY_AUDIO_BITDEPTH_16,   /**< & */
        DSY_AUDIO_BITDEPTH_24,   /**< & */
        DSY_AUDIO_BITDEPTH_LAST, /**< & */
    } dsy_audio_bitdepth;

    /** Setting for each SAI that sets whether the processor is generating the MCLK signal or not. */
    typedef enum
    {
        DSY_AUDIO_SYNC_MASTER, /**< No Crystal */
        DSY_AUDIO_SYNC_SLAVE,  /**< Crystal */
        DSY_AUDIO_SYNC_LAST,   /**< & */
    } dsy_audio_sync;

    /** Each SAI has two datalines, they can independently be configured as inputs or outputs.*/
    typedef enum
    {
        DSY_AUDIO_RX, /**< & */
        DSY_AUDIO_TX, /**< & */
    } dsy_audio_dir;

    /** List of the pins that need to be initialized SIN/SOUT is a bit misleading, and should be turned into A/B since it is possible to configure two inputs or two outputs on a single SAI.*/
    typedef enum
    {
        DSY_SAI_PIN_MCLK, /**< & */
        DSY_SAI_PIN_FS,   /**< & */
        DSY_SAI_PIN_SCK,  /**< & */
        DSY_SAI_PIN_SIN,  /**< & */
        DSY_SAI_PIN_SOUT, /**< & */
        DSY_SAI_PIN_LAST, /**< & */
    } dsy_sai_pin;

    /** List of devices with built in support. Devices not listed here, will need to have initialization done externally. */
    typedef enum
    {
        DSY_AUDIO_NONE,           /**< For unsupported, or custom devices. */
        DSY_AUDIO_DEVICE_PCM3060, /**< & */
        DSY_AUDIO_DEVICE_WM8731,  /**< & */
        DSY_AUDIO_DEVICE_AK4556,  /**< & */
        DSY_AUDIO_DEVICE_LAST,    /**< & */
    } dsy_audio_device;

    /** Index for the several arrays in the sai_handle struct below. */
    enum
    {
        DSY_SAI_1,    /**< & */
        DSY_SAI_2,    /**< & */
        DSY_SAI_LAST, /**< & */
    };

    /** Configuration structure for SAI  contains all above settings, and passes them to internal structure for hardware initialization. */
    typedef struct
    {
        dsy_audio_sai        init;                      /**< & */
        dsy_audio_samplerate samplerate[DSY_SAI_LAST];  /**< & */
        dsy_audio_bitdepth   bitdepth[DSY_SAI_LAST];    /**< & */
        dsy_audio_dir        a_direction[DSY_SAI_LAST]; /**< & */
        dsy_audio_dir        b_direction[DSY_SAI_LAST]; /**< & */
        dsy_audio_sync       sync_config[DSY_SAI_LAST]; /**< & */
        dsy_audio_device     device[DSY_SAI_LAST];      /**< & */

        dsy_gpio_pin sai1_pin_config[DSY_SAI_PIN_LAST]; /**< & */
        dsy_gpio_pin sai2_pin_config[DSY_SAI_PIN_LAST]; /**< & */
    } dsy_sai_handle;

    /** Intializes the SAI peripheral(s) with the specified settings. Pinlists should be arrays of DSY_SAI_PIN_LAST elements 
      \param init &
      \param sr[] Sample rate per chan: 0, 1
      \param bitdepth[] Bitdepth per chan: 0, 1 
      \param sync_config[] & sync config per chan: 0, 1
      \param *sai1_pin_list &
      \param *sai2_pin_list &
  */
    void dsy_sai_init(dsy_audio_sai        init,
                      dsy_audio_samplerate sr[2],
                      dsy_audio_bitdepth   bitdepth[2],
                      dsy_audio_sync       sync_config[2],
                      dsy_gpio_pin*        sai1_pin_list,
                      dsy_gpio_pin*        sai2_pin_list);

    /** Uses the data within *hsai to initialize the peripheral(s) 
    \param hsai &
     */
    void dsy_sai_init_from_handle(dsy_sai_handle* hsai);

#ifdef __cplusplus
}
#endif
#endif //DSY_SAI_H
/** @} */
