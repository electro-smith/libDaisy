# libDaisy Changelog

## v5.3.0

### Features

* driver: Software SPI transport `SSD130x4WireSoftSpiTransport` added for the OLED Display driver. (#551)

### Bug Fixes

* driver: Fixed a compiler error in `Max11300Driver::WriteAnalogPinVolts()`
* driver: Fixed error reading multiple registers at once from the MPC23x17 GPIO expanders (#550)
* seed: Fixed out of range pin definitions for extra GPIO on the Daisy Seed2 DFM (#544)
* patchsm: Fixed issue where updating the audio callback params didn't update control samplerate (#543)

## v5.2.0

### Features

* board: added board support for Noise Engineering legio platform
* audio: added `output_compensation` value to config struct to allow for post-scaling of uneven audio passthru levels.
* util: added a multiply operator to the Color class for scaling a color by a single factor.
* device: Added ICM20948 sensor device driver
* device: Added DPS310 device driver
* device: Added MPR121 device driver
* device: Added APDS9960 device driver
* device: Added TLV493D device driver.
* device: Added neotrellis driver
* device: Added neopixel driver

### Bug fixes

* uart: fixed bug with fifo-dma-receive mode that would result in erratic reads over time. Fixes issues with UART (TRS/DIN) MIDI parsing

## v5.1.0

### Features

* tim: `TimerHandle` now has callbacks each time the Period has elapsed. These can be enabled with `TimerHandle::Config::enable_irq` at Init time.
* bootloader: Working with the bootloader has been simplified. See [the new guide for updates on usage](https://electro-smith.github.io/libDaisy/md_doc_md__a7__getting__started__daisy__bootloader.html)
* usb: `USBHost` class has added support for user callbacks on device connection, disconnection, and when the MSC class becomes active.
* uart: Adds DMA RX and TX modes, similar to how they work on the I2C and SPI.
* uart: Update function names to be more in line with the new DMA / Blocking scheme. 
  * The old methods are wrappers for the new ones to preserve backwards compatibility, but **will be removed in a future version**.
  * Affected functions: `PollReceive`, `PollTx`, `StartRx`, `RxActive`, `FlushRx`, `PopRx`, `Readable`

### Bug Fixes

* util: PersistentStorage class had a bug where calling the `RestoreDefaults` function would cause a crash
* usb: LL HAL files for USB were updated to prevent timing issues when running with optimization
* spi: Add IRQ handlers for SPI2-5. These should work with DMA now.
* midi: bugs related to running status bytes for note off, and single data-byte messages have been resolved

### Other

* build: core/Makefile has had the `-fnortti` flag added to match libDaisy's Makefile
* bootloader: local version of daisy bootloader has been updated to improve stability
* spi: Added examples for blocking TX and DMA TX, added a writeup explaining how to use the SPI on the Daisy
* uart: Adds examples for common modes of communication, both DMA, blocking, FIFO, and mixed.

## v5.0.0

### Breaking Changes

* driver: MAX11300 driver interface changed considerably
* spi: Order of arguments of the `SpiHandle` DMA functions changed
* dma: `SpiHandle` previously used `DMA1_Stream7`, now uses `DMA2_Stream2` and `DMA2_Stream3`

### Features

* spi: `SpiHandle` now has callbacks before and after a DMA transaction starts (can be used for software driven chip select)
* spi: `SpiHandle` now supports simultaneous transmit and receive with DMA using `SpiHandle::DmaTransmitAndReceive()`
* spi: added `MultiSlaveSpiHandle` that allows to connect to multiple SPI slave devices on a shared bus
* driver: MAX11300 now supports multiple chips on a shared bus
* driver: MAX11300 now uses DMA to handle updates without blocking
* driver: MAX11300 now updates the chips continuously until manually stopped
* driver: MAX11300 can now call a user-provided callback after an update is complete
* debugging: added additional debugging aids to the HardFault handler
* gatein: added invert init parameter for reading from different input circuits from the GateIn class.
* ui: added `OnUserInteraction` virtual function to UI framework to allow for tracking user activity

### Bug Fixes

* logger: Added 10ms delay at the end of `StartLog` function. Without this, messages immediatly following the `StartLog` function were getting missed when `wait_for_pc` is set to `true`.
* testing: debugging configuration now uses `lldb` debugging extension to support unit test debugging on macOS with Apple Silicon
* driver: oled_ssd130x.h - Add the SpiHandle:Config struct to SSD130x4WireTransport:Config to allow full access to the SPI peripheral configuration.
* hid: fixed issue in `AnalogControl` where computed coeff could be out of range with certain block sizes
* driver: added missing alternate function pin mappings for SPI2, and UART for pins available on the patch_sm hardware 
* usb: fixed issue with MIDI output from USB
* driver: fixed off-by-one error in qspi erase function.

### Other

* driver: improved debouncing for `Switch`, and `Encoder` classes (limiting debouncing to 1kHz max frequency internally).

### Migrating

#### SPI

```c++
// Old
SpiHandle::Result DmaTransmit(uint8_t*            buff,
                              size_t              size,
                              SpiHandle::CallbackFunctionPtr callback,
                              void*               callback_context);
// New
SpiHandle::Result DmaTransmit(uint8_t*                            buff,
                              size_t                              size,
                              SpiHandle::StartCallbackFunctionPtr start_callback,
                              SpiHandle::EndCallbackFunctionPtr   end_callback,
                              void*                               callback_context);
// Same for DmaReceive and DmaTransmitAndReceive
```

#### MAX11300

```c++
// Old: MAX11300 takes no template arguments
MAX11300 max11300driver;
// New: MAX11300 takes number of chips as template argument
constexpr size_t num_devices = 4;
MAX11300<num_devices> max11300driver;

// Old: constructor only takes a config struct
max11300driver.Init(config);
// New: constructor takes a DMA buffer situated in non-cached and DMA-accessible memory
MAX11300Types::DmaBuffer DMA_BUFFER_MEM_SECTION max11300DmaBuffer;
max11300driver.Init(config, &max11300DmaBuffer);

// Old: most types are inside the MAX11300 classname scope
daisy::MAX11300::PIN_0
daisy::MAX11300::AdcVoltageRange
// New: types are in a separate namespace to keep them independent from the "num_devices" template argument
daisy::MAX11300Types::PIN_0
daisy::MAX11300Types::AdcVoltageRange

// Old: only a single chip was handled by the driver
max11300driver.ConfigurePinAsAnalogRead(daisy::MAX11300::PIN_0, daisy::MAX11300::AdcVoltageRange::NEGATIVE_5_TO_5);
max11300driver.ConfigurePinAsAnalogWrite(daisy::MAX11300::PIN_1, daisy::MAX11300::DacVoltageRange::NEGATIVE_5_TO_5);
// New: each function now takes an additional argument, the index of the chip
max11300driver.ConfigurePinAsAnalogRead(0, daisy::MAX11300Types::PIN_0, daisy::MAX11300Types::AdcVoltageRange::NEGATIVE_5_TO_5);
max11300driver.ConfigurePinAsAnalogWrite(1, daisy::MAX11300Types::PIN_1, daisy::MAX11300Types::DacVoltageRange::NEGATIVE_5_TO_5);

// Old: Update() synchronously updates the chip and blocks until the update is complete
max11300driver.Update(); // blocks
// New: - Start() works asynchronously in the background using DMA
//      - Start() will retrigger updates itself automatically, until stopped by calling Stop()
//      - A callback can be called after each update cycle that was completed successfully
void MyUpdateCompleteCallback(void* context) {
    // The context is the pointer you passed when calling `.Update()`
    // This callback comes from an interrupt, keep is fast.
}
max11300driver.Start(
    &MyUpdateCompleteCallback, // or nullptr if you don't need a callback. default: nullptr
    &someThingThatYouWantToGetPassedToYourCallback // callback context, or nullptr if not needed
);
// you don't have to specify the arguments, then the defaults will be used
max11300driver.Start();
// you can stop the auto update after you started it
max11300driver.Stop();
```

## v4.0.0

### Breaking Changes

* driver: added support for the 0 .. 2.5V ADC range to MAX11300, splitting the `enum VoltageRange` into two enums for the ADC and DAC configurations.

### Features

* driver: added support for the MCP23x17 I/O Expander with I2C transport implementation

### Bug Fixes

* usb: fixed bug where using FatFS and a USB Device class simultaneously would result in a linker error.
  * Shared IRQHandlers for the USB HS peripheral have been moved to sys/system.cpp
* driver: made MAX11300 getter functions `const`
* cmake: changed optimization to `-O0` for Debug builds
* qspi: fixed bug causing one sector erase beyond the given end address

### Other

### Migrating

#### MAX11300

```c++
// Old: Same enum used for DAC and ADC configurations
max11300driver.ConfigurePinAsAnalogRead(daisy::MAX11300::PIN_0, daisy::MAX11300::VoltageRange::NEGATIVE_5_TO_5);
max11300driver.ConfigurePinAsAnalogWrite(daisy::MAX11300::PIN_1, daisy::MAX11300::VoltageRange::NEGATIVE_5_TO_5);

// New: Different enum used for DAC and ADC configurations
max11300driver.ConfigurePinAsAnalogRead(daisy::MAX11300::PIN_0, daisy::MAX11300::AdcVoltageRange::NEGATIVE_5_TO_5);
max11300driver.ConfigurePinAsAnalogWrite(daisy::MAX11300::PIN_1, daisy::MAX11300::DacVoltageRange::NEGATIVE_5_TO_5);
```

## v3.0.0

### Breaking Changes

* fatfs: the `dsy_fatfs_init()` function has been replaced with the `FatFSInterface` class that allows for multi-volume filesystem use with SDMMC and/or USB drives.
  * The previous global variables `SDPath`, `SDFatFS`, `SDFile`, and `retSD` have been removed as they could not be used effectively with the addition of multi-volume support.
  * The path, and FATFS objects can be acquired from the new `FatFSInteface` objects, and any `FIL` object(s) can be placed in AXI SRAM (or any other memory with USB).
* namespace: new namespace "seed" was added for new pinout Daisy Seed pinout mappings.
  * This isn't a breaking change on it's own, but when `using namespace seed;` a variable cannot be named, `seed` else it will throw ambiguity errors.

### Features

* bootloader: the bootloader can now be flashed directly from libDaisy using `make program-boot`
  * apps can be built by overwriting `LDSCRIPT` with one of the new (\_sram.lds, or \_qspi.lds) linker scripts, and adding `-DBOOT_APP` to the `C_DEFS` within the user makefile.
  * once the bootloader is on the device, apps compiled with the above changes can be flashed to the bootloader using `make program-app`
  * this process is subject to change, and will be improved and documented in the coming weeks.
* driver: added support for the MAX11300 ADC/DAC/GPI/GPO device
* usb_midi: added `MidiUsbTransport` class for easy usb midi functionality via `MidiUsbHandler`
* ui: addition of `CanvasDescriptor::screenSaverTimeout` setting turns off display after as many milliseconds. Useful for preventing screen burn-in on sensitive displays like OLEDs.
* usb_host: basic support for Mass Storage USB host class has been added.
* fatfs: interface has been reworked, and now supports mounting SDMMC, and/or USB drives. Multi-volume usage with simultaneous use of both is supported.
* core: added new C++ `Pin` type that is constexpr friendly, and backwards compatible with existing C-style `dsy_gpio_pin` type.
  * the old, `dsy_gpio_pin` and it's accompanying functions should no longer be used, and will be removed in a later version of the library.
* gpio: added new C++ API for access to GPIO
  * the old, `dsy_gpio` and it's accompanying functions should no longer be used, and will be removed in a later version of the library.
* seed: new pinout consts have been added using the new type. This is to simplify addressing pins on Daisy Seed, and the names are shared with the Arduino Integration.
  * These constants live in the `daisy::seed` namespace.
  * Digital pins D0-D30 have been added, these return the equivalent pin as `DaisySeed::GetPin(int pinno)`
  * Analog pins A0-A11 map to the ADC accessible pins labeled as "ADCn" in the pinout diagram.
  * The `Ax` analog pin constants can be used when using digital functions, and vice-versa.

### Bug fixes

* patch_sm: corrected the order of the gate out pins
* sai: fixed occasional output audio channel swap (#268)
* sd_diskio: removed extraneous strobing of unrelated GPIO pin
* patch: fixed seed1.1 compatibility to properly initialize the primary codec due to special 4-channel audio

### Other

* switch: Use `System::GetNow()` rather than the update rate to calculate `TimeHeldMs()`.  
  * This has also been applied to the `Encoder` class (since it uses `Switch` internally).
* usb host: ST Middleware for USB Host support has been added to the Middlewares folder
* fatfs: changed default `FS_LOCK` to 0, allowing for more simultaneously open FIL objects.
* seed: the contents of `DaisySeed::Configure()` have been moved into the `DaisySeed::Init()` function, leaving it empty. This function can now be omitted.
  * the function was left in place to preserve backwards compatibility, and will be removed in a future version.

### Migrating

* switch/encoder: Backwards compatability for Initializing switches/encoders will be maintained until the next breaking change, at which point the `update_rate` argument will be removed from `Switch::Init`, and `Encoder::Init`.
* fatfs: If file locking was being used, `FS_LOCK` will have to be changed back to a non-zero, positive value.

#### FatFS

Existing code using SDMMC for FatFS only requires a small change to replace the previous `dsy_fatfs_init()` function:

```c++
FatFSInterface fsi;

int main(void) {
  // Daisy, SDMMC, etc. initialization unchanged
  // before:
  dsy_fatfs_init();
  . . .
  // now:
  fsi.Init(FatFSInterface::Config::MEDIA_SD);

  // and instead of the global FATFS object you can now:
  FATFS& fs = fsi.GetSDFileSystem();

  // and instead of the global SDPath object you can now:
  const char* rootpath = fsi.GetSDPath();

  // and then file system works as usual now
  f_mount(&fs, rootpath, 1);
  . . .
}
```

#### GPIO

The new `GPIO` class, and `Pin` type have been added to replace the previous C versions.

The C versions will remain in place for some time to support backwards compatibility, and give people time to migrate.

To make this easier, the new `Pin` type can be automatically converted to the old, `dsy_gpio_pin` to use with classes that have not been updated yet (like Switch, Encoder, etc.)

Here's an example comparing the new and old versions of dealing with GPIO (only one of the many initialization methods is shown here):

```c++
//////////////////////////////////////////
// Initialization:
//////////////////////////////////////////
// Old:
dsy_gpio_pin pa1 = {DSY_GPIOA, 1};
dsy_gpio gpio1;
gpio1.mode = DSY_GPIO_OUTPUT_PP;
gpio1.pull = DSY_GPIO_NOPULL;
gpio1.pin = pa1;
dsy_gpio_init(&gpio1);

// New:
Pin pa1 = Pin(PORTA, 1);
GPIO gpio1;
gpio1.Init(pa1, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL);

//////////////////////////////////////////
// Read
//////////////////////////////////////////
// Old:
uint8_t state = dsy_gpio_read(&gpio1);
// New:
bool state = gpio1.Read();

//////////////////////////////////////////
// Write
//////////////////////////////////////////
// Old:
dsy_gpio_write(&gpio1, 1); // HIGH
dsy_gpio_write(&gpio1, 0); // LOW
// New:
gpio1.Write(true); // HIGH
gpio1.Write(false); // LOW

//////////////////////////////////////////
// Toggle
//////////////////////////////////////////
// Old:
dsy_gpio_toggle(&gpio1);
// New:
gpio1.Toggle();
```

## v2.0.1

### Breaking Changes

### Features

### Bug fixes

* qspi: fixed bug with GetData() that wouldn't return correct data when passed actual address instead of normalized offset (i.e. >= 0x90000000)
* sdram: fixed occasional hard fault caused by `RPDelay`, which is now set to 16 (same as in v1.0.0 and earlier)
* patch_sm: fixed integer overflow error with `VoltageToCode` method

### Other

## v2.0.0

### Breaking Changes

* qspi: updated from C to C++, and fixed up the API to be cleaner, and simpler to use
* sdram: updated from C to C++
  * Create SdramHandle class. Gets rid of configurable pin and hardcodes it instead.

### Features

* adc: added initialization mappings for pins ADC1_INP12 (PC2) and ADC1_INP13 (PC3) (Not accessible on Daisy Seed)
* board: added support files for upcoming Daisy Patch SM hardware
* rng: added new Random module that provides access to the hardware True Random Number Generator
* spi: added DMA Transactions (same type of queue system as I2C) to the SPI Handle class.
* util: added new PersistentStorage class for storing/recalling data between power cycles
* util: added new VoctCalibration helper class for calibrating ADC inputs
* seed: added support for Daisy Seed 1.1 (aka Daisy Seed rev5) hardware. Pin-compatible, with same form factor. WM8731 Codec instead of AK4556.
* bootloader: added `program-app` make target to upload code to the daisy bootloader

### Bug fixes

* adc: fixed bug with ADC where designs with more than 8 channels AND a mux would halt the ADC.
* sdram: changed SdramTiming.RPDelay to 1 to avoid underflow bug.

### Other

* util: added deinits and memory-aware inits for bootloader compatibility

### Migrating

#### QSPI

```c++
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
```

#### Sdram

This is only applicable if you had a seed-level board that needed to initialize external SDRAM.

```cpp
//Init
//Old:
dsy_gpio_pin *pin_group;
dsy_sdram_handle sdram_handle;
sdram_handle.state             = DSY_SDRAM_STATE_ENABLE;
pin_group                      = sdram_handle.pin_config;
pin_group[DSY_SDRAM_PIN_SDNWE] = dsy_pin(DSY_GPIOH, 5);
dsy_sdram_init(&sdram_handle);

//New:
SdramHandle sdram;
sdram.Init();
```

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
