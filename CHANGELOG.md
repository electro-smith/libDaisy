# libDaisy Changelog

## Unreleased

### Breaking Changes
Gpio: Update from c to cpp.
Gpio: Create GPIO and Pin classes.
Gpio: Update hal_map from c to cpp.

### Features

### Bug Fixes
### Other
Gpio: Update all modules to use GPIO class rather than old dsy_gpio.

### Migrating

#### GPIO:
```
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

## v0.1.0

Initial Release

