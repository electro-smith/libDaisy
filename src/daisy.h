#ifndef DSY_LIBDAISY_H
#define DSY_LIBDAISY_H

#include <stdint.h>
#include "daisy_core.h"
#include "version.h"

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
#include "hid/switch3.h"
#include "hid/ctrl.h"
#include "hid/gatein.h"
#include "hid/parameter.h"
#include "hid/usb.h"
#include "hid/logger.h"
#include "hid/usb_host.h"
#include "per/sai.h"
#include "per/sdmmc.h"
#include "per/spi.h"
#include "per/rng.h"
#include "hid/disp/display.h"
#include "hid/disp/oled_display.h"
#include "hid/disp/graphics_common.h"
#include "hid/wavplayer.h"
#include "hid/led.h"
#include "hid/rgb_led.h"
#include "dev/sr_595.h"
#include "dev/codec_pcm3060.h"
#include "dev/codec_wm8731.h"
#include "dev/lcd_hd44780.h"
#include "dev/mcp23x17.h"
#include "dev/max11300.h"
#include "ui/ButtonMonitor.h"
#include "ui/PotMonitor.h"
#include "ui/UI.h"
#include "ui/UiEventQueue.h"
#include "ui/AbstractMenu.h"
#include "ui/FullScreenItemMenu.h"
#include "util/scopedirqblocker.h"
#include "util/CpuLoadMeter.h"
#include "util/FIFO.h"
#include "util/FixedCapStr.h"
#include "util/MappedValue.h"
#include "util/PersistentStorage.h"
#include "util/Stack.h"
#include "util/VoctCalibration.h"
#include "util/WaveTableLoader.h"
#include "util/WavWriter.h"
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

/** @defgroup ui UI
        @brief UI system. UI event queue, event readers, etc.
    @{ 
    */
/** @} END UI */

/** @defgroup utility UTILITY
        @brief General utilities. Ringbuffers, LED colors, OLED stuff, etc.
    @{ 
    */
/** @} END UTILITY */

/** @} END LIBDAISY*/
