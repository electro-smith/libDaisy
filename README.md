# ES_libdaisy

- [libdaisy](#libdaisy)
- [daisysp](#daisysp)
- [examples](#examples)
- [cube](#cube)
- [resources](#resources)
- [how to flash](#how-to-flash)

## libdaisy

Hardware abstraction layer, including drivers for peripherals included on the extended Daisy family of boards.

**The model for this is still being worked out. So there may be some API breaking changes in the next week or two as everything gets moved into place**

Keeping that in mind, in the included simple examples there will either be an "init" function, or no real use of the main loop to keep it easy to maintain them during this period of change.

### Functional Drivers

- System Driver for initializing Kernal/Periph Clocks, PLLs, etc.
- SDRAM for 64MB Alliance SDRAM chip
- QSPI for 4MB NOR FLASH IS25LP064D
- QSPI for 1MB NOR FLASH IS25LP080D
- Stereo Audio for Wolfson WM8731
- Stereo Audio for TI PCM3060

### Partial/Incomplete Drivers

- ADC for Internal 8-11x 16-bit oversample-able ADCs.
- PCA9685 LED Driver via I2C (16-channel PWM)

### Drivers coming soon:

- DAC for 2-channel 12-bit DAC
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

These require arm-none-eabi-gcc to compile, and make is used for the build.

They also currently require openocd to program to the Daisy Seed via an stlink or similar JTAG programmer.

- simpleverb - Stereo ReverbSC with time, lpfreq, send amount, and dry level controls via the knobs.
- simplebypass - Simple Audio Callback with the input being copied to the output.

To build any of the examples, navigate to their folder:

```
cd examples/simpleverb
```

and use make to build, and program the board.

```
# build
make
# flash program to the Daisy Seed
make program
```

A debug client/server is setup for use in the makefile (when connected via JTAG), though it hasn't been tested on this platform yet (for this ddd is used, but that can easily be replaced with a gdb debug client of your choice).

Very shortly, a `make program-dfu` will be available to use to program the Daisy via USB.

## cube

STM32CubeMX is a configurator, and code generator for STM32.

Included are the cube files for the boards to generate code.

This can be helpful for generating initialization code, checking the clock configuration, etc. 

Generated code uses ST's HAL and/or LL drivers. These are currently the basis for libdaisy as well, though that may also change.

An entire copy of the ST HAL, as well as Middleware for CDC USB Device Class, and FatFS are included in the libdaisy/ folder.

## resources

The following resources are necessary for either compiling, or programming the Daisy Seed in one way or another.

- make
- [arm-none-eabi toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) - GNU Embedded compiler tools -- currently using Version 8-2019-q3-update.
- [openocd](http://openocd.org/) - "Open On-Chip Debugger" for connecting to, programming, and debugging embedded hardware via JTAG, and other protocols. You can install via your favorite package manager, or follow links from the site for Windows.
- [dfu-util](http://dfu-util.sourceforge.net/index.html) - Device Firmware Upgrade Utilities. Install via a pacakge manager, or follow links to binaries for windows, etc.


## how to flash

### JTAG // ST-Link

If you have an ST-Link, and a 2x20->2x10mini adapter (Links will be added soon). You can connect it with the redstripe facing up toward the white strip on the 2x10 male connector on the top of the daisy seed.

this is the "default" method of programming the daisy seed.

running `make program` from any of the examples will attempt to program the chip's flash via JTAG through openocd.

### USB

If you don't have the JTAG programmer, fear not! A USB Micro cable connected to the daisy seed is all you need.

To enter DFU mode, hold both the RESET, and BOOT buttons on the daisy seed while its powered.

Then let go of just the RESET button. This will cause the chip to reset into the system bootloader. The BOOT button can now be released as well.

Now you can run `make program-dfu` from any example folder. You will see a short Download Progress bar appear in your terminal.

Once it says "File downloaded successfully" the reset button will need to be pressed again to reboot the device, and start the newly flashed program.

For the time being there are a few oddities with this method:
- an error, "Error sending dfu abort request" will always output. This is benign.
- The second press of reset should not be necessary, and I'll try to add some sort of code that generates a proper reset over USB since dfu-util seems to do it improperly (I believe this has been fixed in the stm32-duino project).



