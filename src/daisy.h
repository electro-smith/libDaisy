#ifndef DSY_LIBDAISY_H
#define DSY_LIBDAISY_H

#include <stdint.h>
#include "daisy_core.h"

/** @addtogroup boards
    @{
*/

#include "sys/system.h"
#include "per/qspi.h"
#include "per/dac.h"
#include "per/gpio.h"
#include "per/tim.h"
#include "dev/leddriver.h"
#include "dev/sdram.h"
#include "dev/sr_4021.h"
#include "hid/audio.h"
#include "util/unique_id.h"
#ifdef __cplusplus
#include "per/i2c.h"
#include "per/adc.h"
#include "per/uart.h"
#include "hid/midi.h"
#include "hid/encoder.h"
#include "hid/switch.h"
#include "hid/ctrl.h"
#include "hid/gatein.h"
#include "hid/parameter.h"
#include "hid/usb.h"
#include "per/sai.h"
#include "per/sdmmc.h"
#include "per/spi.h"
#include "hid/oled_display.h"
#include "hid/wavplayer.h"
#include "hid/led.h"
#include "hid/rgb_led.h"
#include "dev/sr_595.h"
#include "util/scopedirqblocker.h"
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
