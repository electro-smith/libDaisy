# libDaisy Changelog

## Unreleased

### Breaking Changes
- Gpio: Update from c to cpp.
- Gpio: Create GPIO and Pin classes.
- Gpio: Update hal_map from c to cpp.
- Sdram: Create SdramHandle class. Gets rid of configurable pin and hardcodes it instead.

### Features
- Sdram: Update  from c to cpp.

### Bug Fixes
- Change SdramTiming.RPDelay to 1 to avoid underflow bug.

### Other
- Gpio: Update all modules to use GPIO class rather than old dsy_gpio.

### Migrating

#### GPIO:
``` c++
// Init
  // OLD:
  mygpio.mode = DSY_GPIO_MODE_INPUT;
  mygpio.pin = {DSY_GPIOA, 1};
  dsy_gpio_init(&mygpio);
  // NEW: 
  GPIO::Config conf;
  conf.mode = GPIO::Config::Mode::INPUT;
  conf.pin = {Port::DSY_GPIOA, 1};
  mygpio.Init(gpio_conf);

// Write
  // OLD:
  dsy_gpio_write(&mygpio, 1);
  // NEW:
  mygpio.Write(true);
  
// Read
    //Old:
    dsy_gpio_read(&mygpio);
    // New:
    mygpio.Read();

// Toggle
    //Old:
    dsy_gpio_toggle(&mygpio);
    // New:
    mygpio.Toggle();
```
#### Sdram:
``` cpp
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

## v0.1.0

Initial Release

