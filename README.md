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

- SDRAM for 64MB Alliance SDRAM chip
- QSPI for 4MB IS25LP064D
- QSPI for 1MB IS25LP080D
- Stereo Audio for Wolfson WM8731
- Stereo Audio for TI PCM3060

## libdsysp

Digital Signal Processing library for the extended Daisy family of boards.

This library is intentionally kept separate so that it can be compiled for x86 as well as ARM. 

This will allow for development and prototyping on a computer before running it on hardware.
