

# ES_libdaisy
- [Overview](#overview)
- [libDaisy](#libdaisy)
- [DaisySP](#daisysp)
- [How to Build](https://github.com/andrewikenberry/ES_libdaisy/wiki/How-To-Build)
- [How to Flash](https://github.com/andrewikenberry/ES_libdaisy/wiki/How-To-Flash)
- [Examples](https://github.com/andrewikenberry/ES_libdaisy/tree/master/examples)
- [Cube](#cube)

## Overview

Daisy is a open source sound computer platform...

## libDaisy

Hardware abstraction layer, including drivers for peripherals included on the extended Daisy family of boards.

Keeping that in mind, in the included simple examples there will either be an "init" function, or no real use of the main loop to keep it easy to maintain them during this period of change.

## DaisySP

Digital Signal Processing library for the Daisy family of boards.

This library is intentionally kept separate so that it can be compiled outside of an ARM context. 

This will allow for development and prototyping on a computer before running it on hardware.

## Cube

STM32CubeMX is a configurator, and code generator for STM32.

Included are the cube files for the boards to generate code.

This can be helpful for generating initialization code, checking the clock configuration, etc. 

Generated code uses ST's HAL and/or LL drivers. These are currently the basis for libdaisy as well, though that may also change.

An entire copy of the ST HAL, as well as Middleware for CDC USB Device Class, and FatFS are included in the libdaisy/ folder.
