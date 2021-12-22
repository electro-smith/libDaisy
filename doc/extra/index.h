#pragma once
namespace daisy {
// Define the groups to be used throughout all of libdaisy for 
// automated documentation
/** @defgroup libdaisy LIBDAISY
 *  @brief Daisy Hardware Library 
 *  @{ // Start libdaisy
 *      @defgroup boards BOARDS
 *      @brief board support files for official hardware
 *      @defgroup human_interface HUMAN_INTERFACE
 *      @brief Human Interface Components (Switch, Led, AnalogControl, AudioHandle, etc.)
 *      @{ // start hid
 *          @defgroup audio AUDIO
 *          @brief Audio API
 * 
 *          @defgroup controls CONTROLS
 *          @brief Switches, Encoders, and other Physical Inputs
 * 
 *          @defgroup feedback FEEDBACK
 *          @brief LEDs, and other physical outputs.
 * 
 * 					@defgroup hid_logging LOGGING
 * 					@brief Logging to external ports (Serial, JTAG, etc.)
 * 
 * 					@defgroup midi MIDI
 * 					@brief MIDI handlers, transports, and message types
 * 
 *      @} //end hid
 *      @defgroup peripheral PERIPHERAL
 *      @brief control over internal mechanisms within MCU (GPIO, AdcHandle, SpiHandle, UartHandler, etc.)
 *      @{ // start peripheral
 * 					@defgroup serial SERIAL
 * 					@brief Serial Communications (i.e. SPI, UART, I2C, etc.)
 * 
 * 				  @defgroup per_analog ANALOG
 * 					@brief Analog Peripherals (ADC, DAC)
 *      @} // end peripheral
 * 			@defgroup system SYSTEM
 * 			@brief Device configuration, DMA, clocks, etc.
 * 			@defgroup device DEVICE
 * 			@brief Externally connected device drivers (Shift Registers, LED Drivers, Codecs, etc.)
 * 			@{ // start device
 * 					@defgroup shiftregister SHIFTREGISTER
 * 					@brief Digital Shift Registers
 * 
 * 					@defgroup flash FLASH
 * 					@brief External Flash Memory
 * 
 * 					@defgroup codec CODEC
 * 					@brief Audio Codecs
 * 
 * 					@defgroup led LED
 * 					@brief LED Driver devices
 * 
 * 					@defgroup sdram SDRAM
 * 					@brief SDRAM devices
 * 			@} // end device
 * 			@defgroup ui UI
 * 			@brief User Interface, UI Event Queue, Event readers, etc.
 * 			@defgroup utility UTILITY
 * 			@brief General Utilities (Ringbuffers, FIFOs, LED Colors, etc.)
 *  @} // End libdaisy
 */
}