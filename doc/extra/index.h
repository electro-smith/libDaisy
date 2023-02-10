#pragma once
namespace daisy {
// Define the groups to be used throughout all of libdaisy for
// automated documentation
/** @defgroup libdaisy LIBDAISY
 *  @brief Daisy Hardware Library
 *
 *  @cond IGNORE // start libdaisy @endcond
 *
 *  @defgroup boards BOARDS
 *  @brief board support files for official hardware
 *  @ingroup libdaisy
 *
 *  @defgroup human_interface HUMAN_INTERFACE
 *  @brief Human Interface Components (Switch, Led, AnalogControl, AudioHandle, etc.)
 *  @ingroup libdaisy
 *
 *  @cond IGNORE // start hid @endcond
 *
 *  @defgroup audio AUDIO
 *  @brief Audio API
 *  @ingroup human_interface
 *
 *  @defgroup controls CONTROLS
 *  @brief Switches, Encoders, and other Physical Inputs
 *  @ingroup human_interface
 *
 *  @defgroup feedback FEEDBACK
 *  @brief LEDs, and other physical outputs.
 *  @ingroup human_interface
 *
 *  @defgroup hid_logging LOGGING
 *  @brief Logging to external ports (Serial, JTAG, etc.)
 *  @ingroup human_interface
 *
 *  @defgroup midi MIDI
 *  @brief MIDI handlers, transports, and message types
 *  @ingroup human_interface
 *
 *  @cond IGNORE // end hid @endcond
 *
 *  @defgroup peripheral PERIPHERAL
 *  @brief control over internal mechanisms within MCU (GPIO, AdcHandle, SpiHandle, UartHandler, etc.)
 *  @ingroup libdaisy
 *
 *  @cond IGNORE // start peripheral @endcond
 *
 *  @defgroup serial SERIAL
 *  @brief Serial Communications (i.e. SPI, UART, I2C, etc.)
 *  @ingroup peripheral
 *
 *  @defgroup per_analog ANALOG
 *  @brief Analog Peripherals (ADC, DAC)
 *  @ingroup peripheral
 *
 *  @cond IGNORE // end perhipheral @endcond
 *
 *  @defgroup system SYSTEM
 *  @brief Device configuration, DMA, clocks, etc.
 *  @ingroup libdaisy
 *
 *  @defgroup device DEVICE
 *  @brief Externally connected device drivers (Shift Registers, LED Drivers, Codecs, etc.)
 *  @ingroup libdaisy
 *
 *  @cond IGNORE // start device @endcond
 *
 *  @defgroup shiftregister SHIFTREGISTER
 *  @brief Digital Shift Registers
 *  @ingroup device
 *
 *  @defgroup flash FLASH
 *  @brief External Flash Memory
 *  @ingroup device
 *
 *  @defgroup codec CODEC
 *  @brief Audio Codecs
 *  @ingroup device
 *
 *  @defgroup led LED
 *  @brief LED Driver devices
 *  @ingroup device
 *
 *  @defgroup sdram SDRAM
 *  @brief SDRAM devices
 *  @ingroup device
 *
 *  @cond IGNORE // end device @endcond
 *
 *  @defgroup ui UI
 *  @brief User Interface, UI Event Queue, Event readers, etc.
 *  @ingroup libdaisy
 *
 *  @defgroup utility UTILITY
 *  @brief General Utilities (Ringbuffers, FIFOs, LED Colors, etc.)
 *  @ingroup libdaisy
 *
 *  @cond IGNORE // end libdaisy @endcond
 */
}
