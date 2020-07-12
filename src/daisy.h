#ifndef DSY_LIBDAISY_H
#define DSY_LIBDAISY_H

#include <stdint.h>
#include "daisy_core.h"


#define FBIPMAX 0.999985f             /**< close to 1.0f-LSB at 16 bit */
#define FBIPMIN (-FBIPMAX)            /**< - (1 - LSB) */
#define S162F_SCALE 3.0517578125e-05f /**< 1 / (2** 15) */
#define F2S16_SCALE 32767.0f          /**< (2 ** 15) - 1 */
#define F2S24_SCALE 8388608.0f        /**< 2 ** 23 */
#define S242F_SCALE 1.192092896e-07f  /**< 1 / (2 ** 23) */
#define S24SIGN 0x800000              /**< 2 ** 23 */

/** @addtogroup boards
    @{
*/

/** 
    Scales float by 1/(2 ^ 15)
    \param x Number to be scaled.
    \return Scaled number.
*/
FORCE_INLINE float s162f(int16_t x)
{
    return (float)x * S162F_SCALE;
}

/**
   &
*/
FORCE_INLINE int16_t f2s16(float x)
{
    x = x <= FBIPMIN ? FBIPMIN : x;
    x = x >= FBIPMAX ? FBIPMAX : x;
    return (int32_t)(x * F2S16_SCALE);
}

/**
#
 */
FORCE_INLINE float s242f(int32_t x)
{
    x = (x ^ S24SIGN) - S24SIGN; //sign extend aka ((x<<8)>>8)
    return (float)x * S242F_SCALE;
}
/**
   &
 */
FORCE_INLINE int32_t f2s24(float x)
{
    x = x <= FBIPMIN ? FBIPMIN : x;
    x = x >= FBIPMAX ? FBIPMAX : x;
    return (int32_t)(x * F2S24_SCALE);
}


#include "sys_system.h"
#include "per_qspi.h"
#include "per_dac.h"
#include "per_gpio.h"
#include "per_sai.h"
#include "per_tim.h"
#include "dev_leddriver.h"
#include "dev_sdram.h"
#include "dev_sr_4021.h"
#include "hid_audio.h"
#include "util_unique_id.h"
#ifdef __cplusplus
#include "per_i2c.h"
#include "per_adc.h"
#include "per_uart.h"
#include "hid_midi.h"
#include "hid_encoder.h"
#include "hid_switch.h"
#include "hid_ctrl.h"
#include "hid_gatein.h"
#include "hid_parameter.h"
#include "hid_usb.h"
#include "per_sdmmc.h"
#include "per_spi.h"
#include "hid_oled_display.h"
#include "hid_wavplayer.h"
#include "hid_led.h"
#include "hid_rgb_led.h"
#include "dev_sr_595.h"
#endif
#endif

/** @} */

//Define the groups to be used throughout all of libdaisy

/** @defgroup libdaisy LIBDAISY
    @brief The daisy library
    @{ */

/** @defgroup human_interface HUMAN_INTERFACE
    @brief Interface with the world.
    @{ */

/** @defgroup audio AUDIO
        @brief Embedded Audio Engine */

/** @defgroup controls CONTROLS
        @brief Hardware Controls */

/** @defgroup feedback FEEDBACK
        @brief Screens, leds, etc. */

/** @defgroup  external EXTERNAL
        @brief External interface devices */

/** @} END HID */

/** @defgroup peripheral PERIPHERAL
    @brief Peripheral devices, not meant for human interaction.
    @{ 
    */

/** @defgroup  serial SERIAL
        @brief Serial Communications */

/** @defgroup  analog_digital_conversion ANALOG_DIGITAL_CONVERSION
        @brief Convert from digital to analog, or vice-versa */

/** @defgroup  other OTHER
        @brief GPIO, timers, and SDMMC */

/** @} END PERIPHERAL */

/** @defgroup system SYSTEM
    @brief Deals with system. DMA, clocks, etc.
    @{ 
    */
/** @} END SYSTEM */

/** @defgroup device DEVICE
        @brief Low level devices. Led drivers, codecs, etc.
    @{ 
    */
/** @defgroup  shiftregister SHIFTREGISTER
        @brief Digital shift registers*/

/** @defgroup  flash FLASH
        @brief Flash memory */

/** @defgroup codec CODEC
        @brief Audio codecs */

/** @defgroup  led LED
        @brief LED driver devices */

/** @defgroup  sdram SDRAM
        @brief SDRAM devices */

/** @} END DEV */

/** @defgroup boards BOARDS
    @brief Daisy devices. Pod, seed, etc.
    @{ 
    */
/** @} END BOARDS */

/** @defgroup utility UTILITY
        @brief General utilities. Ringbuffers, LED colors, OLED stuff, etc.
    @{ 
    */
/** @} END UTILITY */

/** @} END LIBDAISY*/
