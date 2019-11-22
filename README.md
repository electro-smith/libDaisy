

# Daisy - Open Source Sound Computer
- [Overview](#overview)
- [Programming the Daisy](#programming-the-daisy)
- [libDaisy](#libdaisy)
- [DaisySP](#daisysp)
- [How to Build](https://github.com/andrewikenberry/ES_libdaisy/wiki/How-To-Build)
- [How to Flash](https://github.com/andrewikenberry/ES_libdaisy/wiki/How-To-Flash)
- [Examples](https://github.com/andrewikenberry/ES_libdaisy/tree/master/examples)
- [Cube](#cube)

## Overview

Daisy is an open source sound computer. It provides an embedded platform for high fidelity/low latency audio processing without the complexity of designing a system from scratch. 

## Programming the Daisy
Daisy can be programmed in a number of languages / environments. Here are the ones that are currently operational with examples and documentation(quantity of examples/docs may vary):

- Arduino IDE

- C

Coming soon:

- Max/MSP Gen~

- FAUST

- Pure Data export

## libDaisy

Library for hardware abstraction and peripheral drivers including MIDI, USB, ADC, etc. 

License: MIT


## DaisySP

DSP library for the Daisy family of boards.

License: MIT

This library is intentionally kept separate so that it can be compiled outside of an ARM context. 

This will allow for development and prototyping on a computer before running it on hardware.

## Cube

STM32CubeMX is a configurator, and code generator for STM32.

Included are the cube files for the boards to generate code.

This can be helpful for generating initialization code, checking the clock configuration, etc. 

Generated code uses ST's HAL and/or LL drivers. These are currently the basis for libDaisy as well, though that may also change.

An entire copy of the ST HAL, as well as Middleware for CDC USB Device Class, and FatFS are included in the libdaisy/ folder.
