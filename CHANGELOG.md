# libDaisy Changelog

## Unreleased

### Breaking Changes

* qspi: updated from C to C++, and fixed up the API to be cleaner, and simpler to use

### Features

* adc: added initialization mappings for pins ADC1_INP12 (PC2) and ADC1_INP13 (PC3) (Not accessible on Daisy Seed)
* board: added support files for upcoming Daisy Patch SM hardware
* rng: added new Random module that provides access to the hardware True Random Number Generator
* spi: added DMA Transactions (same type of queue system as I2C) to the SPI Handle class.

### Bug fixes

* adc: fixed bug with ADC where designs with more than 8 channels AND a mux would halt the ADC.

### Other

### Migrating

#### QSPI
~~~c++
DaisySeed hw;
// ...

// Old
hw.qspi_handle.mode = DSY_QSPI_MODE_INDIRECT_POLLING;
dsy_qspi_init(&hw.qspi_handle);
dsy_qspi_erase(address, address + sector_size);
dsy_qspi_write(address, size, (uint8_t*)&some_data);

// New -- qspi mode is automatically handled
hw.qspi.Erase(address, address + sector_size);
hw.qspi.Write(address, size, (uint8_t*)&some_data);
~~~

## v1.0.0

### Breaking Changes

* move MidiEvent and related data to `daisy` namespace.

### Features

* string: allow more integer types in `FixedCapStr::AppendInt()`
* fifo/stack: Add new methods for searching elements
* fifo/stack: Reduce binary size
* RgbLed: Added individual setters for each color

### Bug fixes

* midi: SysEx messages that overflow stop reading data until rx sysexstop. Previously overflowed sysex would cause junk messages.
* midi: NoteOns of velocity 0 cause NoteOffs.
* usb-serial: fix RX callback function being overwritten

### Other

* test: add unit testing for midi parser.  
* tests: add tests for `FIFO`
* docs: Update TODO comment in `uart.h` to reflect most recent uart update.
* ci: add filters to the workflows
* ci: add workflow that builds libDaisy with CMake
* build: small fixes in CMakeLists.txt
* build: report detailed memory usage when linking

### Migrating

With `using namespace daisy` (as typical in example programs) no changes necessary. Otherwise:

```c++
// Old
MidiEvent event;

// New
daisy::MidiEvent event;
```

## v0.1.0

Initial Release
