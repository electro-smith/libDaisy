# libDaisy Changelog

## Unreleased

* Update TODO comment in uart.h to reflect most recent uart update.
* Add MidiEvent and related data to daisy namespace.
* Update qspi API from C to C++.

### Migrating QSPI example

~~~ C++
DaisySeed hw;
// ...

// Old
hw.qspi_handle.mode = DSY_QSPI_MODE_INDIRECT_POLLING;
dsy_qspi_init(&hw.qspi_handle);
dsy_qspi_erase(address, address + sector_size);

// New
auto config = hw.qspi.GetConfig();
config.mode = QSPIHandle::Config::Mode::INDIRECT_POLLING;
hw.qspi.Init(config);
hw.qspi.Erase(address, address + sector_size);
~~~

## v0.1.0

Initial Release