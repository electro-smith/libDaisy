

# ES_libdaisy
- [overview](
- [libdaisy](#libdaisy)
- [daisysp](#daisysp)
- [how to build](https://github.com/andrewikenberry/ES_libdaisy/wiki/How-To-Build)
- [how to flash](https://github.com/andrewikenberry/ES_libdaisy/wiki/How-To-Flash)
- [examples](#examples)
- [cube](#cube)
- [resources](#resources)

## libdaisy

Hardware abstraction layer, including drivers for peripherals included on the extended Daisy family of boards.

**The model for this is still being worked out. So there may be some API breaking changes in the next week or two as everything gets moved into place**

Keeping that in mind, in the included simple examples there will either be an "init" function, or no real use of the main loop to keep it easy to maintain them during this period of change.

### Functional Drivers

- System Driver for initializing Kernal/Periph Clocks, PLLs, etc.
- SDRAM for 64MB Alliance SDRAM chip
- QSPI for 8MB NOR FLASH IS25LP064D
- QSPI for 1MB NOR FLASH IS25LP080D
- Stereo Audio for Wolfson WM8731
- Stereo Audio for TI PCM3060
- DAC for 2-channel 12-bit DAC
- ADC for Internal 8-11x 16-bit oversample-able ADCs.
- PCA9685 LED Driver via I2C (16-channel PWM)
- Switch interface with debouncing, and edge checking. 
- generic GPIO handling.

### Drivers coming soon:


- SDMMC for FATFS filesystem on SD cards. (1-bit and 4-bit)
- USB CDC (VCOM) driver for both USB Peripherals (Built-in and Pinout access)
- MIDI via UART driver
- GPIO Driver for 32x on board GPIO.
- PWM output driver for several of the onboard pins

## daisysp

Digital Signal Processing library for the Daisy family of boards.

This library is intentionally kept separate so that it can be compiled outside of an ARM context. 

This will allow for development and prototyping on a computer before running it on hardware.

### Tested modules:

- decimator
- noise
- reverbsc
- svf

### Untested modules:

- adenv
- nlfilt
- oscillator

### Incomplete modules:

- pstream/pvs 

### Coming Soon

- clock
- delay
- line/lineseg/transeg/curve/etc.
- more filters
- distortions

## examples

There are a few simple examples included.

- simpleverb - Stereo ReverbSC with time, lpfreq, send amount, and dry level controls via the knobs.
- simplebypass - Simple Audio Callback with the input being copied to the output.

A debug client/server is setup for use in the makefile (when connected via JTAG), though it hasn't been tested on this platform yet (for this ddd is used, but that can easily be replaced with a gdb debug client of your choice).

## cube

STM32CubeMX is a configurator, and code generator for STM32.

Included are the cube files for the boards to generate code.

This can be helpful for generating initialization code, checking the clock configuration, etc. 

Generated code uses ST's HAL and/or LL drivers. These are currently the basis for libdaisy as well, though that may also change.

An entire copy of the ST HAL, as well as Middleware for CDC USB Device Class, and FatFS are included in the libdaisy/ folder.
