# libDaisy Changelog

## Unreleased

### Breaking Changes
- Sdram: Create SdramHandle class. Gets rid of configurable pin and hardcodes it instead.

### Features
- Sdram: Update  from c to cpp.

### Bug Fixes

### Migrating

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
```


## v0.1.0

Initial Release

