# libDaisy Changelog

## Unreleased

### Breaking Changes

### Features

* adc: added initialization mappings for pins ADC1_INP12 (PC2) and ADC1_INP13 (PC3) (Not accessible on Daisy Seed)

### Bug fixes

### Other

### Migrating

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
