# libDaisy

libDaisy is a C++ hardware support library for the electrosmith Daisy platform.

the contents of libDaisy fall into several categories, and levels:

* [boards](#link-to-group): these are user-facing APIs for interfacing directly with the official Daisy hardware
* [hid](#link-to-group): these are classes that a user is most likely to directly interact with (switches, encoders, analog controls, etc.)
* [dev](#link-to-group): these are classes that define an interface to a particular externally connected device (shift registers, LED drivers, etc.)
* [per](#link-to-group): these are classes that wrap APIs for access to the microcontrollers internal peripherals (SPI, UART, I2C, etc.)
* [ui](#link-to-group): these are classes to handle a user interface (Event Queues, Control Monitors, Display Management)
* [sys](#link-to-group): these are classes and interfaces for managing the lower levels of the hardware (initializing clock trees, DMA, linking filesystem drivers, etc.)
* [util](#link-to-group): classes and other structures that can be useful on their own, or used within other classes in libDaisy (CPU Load Profiling, FIFOs, diskio, Persistent Storage, etc.)

## Working with Daisy

* [Setting up the development environment]()
* [Creating a new Project]()
* [Working with GPIO](Working-With-GPIO.md)
* [Code Conventions and Common Types]()
* [Printing with USB Serial](Printing-With-USB.md)
* [Reading and Writing files to an SD Card or USB Drive]()
* [Using the external 64MB of SDRAM]()
* [Working with Audio]()
* [Working with the ADC Inputs](Working-With-ADC.md)
* [Working with MIDI]()
* [Running and writing Unit Tests](Unit-Testing.md)

## Troubleshooting

Report bugs, typos, errors, etc. [here on Github]()