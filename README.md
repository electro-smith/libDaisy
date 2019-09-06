# ES_libdaisy

## libdaisy

Hardware abstraction layer, including drivers for peripherals included on the extended Daisy family of boards.

Supported Boards:

- ES Daisy Rev3
- ES Daisy Seed Rev1
- 2hp Audio BB Rev4
- 2hp Audio BB Rev5
- ES Eurorack Tester Rev3

Functional Drivers

- System Driver for initializing Kernal/Periph Clocks, PLLs, etc.
- SDRAM for 64MB Alliance SDRAM chip
- QSPI for 4MB NOR FLASH IS25LP064D
- QSPI for 1MB NOR FLASH IS25LP080D
- Stereo Audio for Wolfson WM8731
- Stereo Audio for TI PCM3060

Drivers coming soon:

- ADC for Internal 8-11x 16-bit oversample-able ADCs.
- DAC for 2-channel 12-bit DAC
- USB CDC (VCOM) driver for both USB Peripherals (Built-in and Pinout access)
- MIDI via UART driver
- PCA9685 LED Driver via I2C (16-channel PWM)
- GPIO Driver for 32x on board GPIO.
- PWM output driver for several of the onboard pins


## libdsysp

Digital Signal Processing library for the extended Daisy family of boards.

This library is intentionally kept separate so that it can be compiled for x86 as well as ARM. 

This will allow for development and prototyping on a computer before running it on hardware.
