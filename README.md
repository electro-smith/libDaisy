# libdaisy

Multi-layer hardware abstraction library for Daisy Product family

On STM32H7 MCUs

Lower-levels use STM32 HAL (local copy w/ modifications in Drivers/)

Prefixes and their meanings:

- sys - System level configuration (clocks, dma, etc.)
- per - Peripheral level, internal to MCU (i2c, spi, etc.)
- dev - External device support (external flash chips, DACs, codecs, etc.)
- hid - User level interface elements (encoders, switches, audio, etc.)
- daisy - core API files (specific boards, platforms have extended user APIs that configure libdaisy more below).

----

# Using libdaisy

Due to the amount of hardware configuration and flexibility of the daisy platform, (in the present, and the future), a user can use libdaisy to define their own custom hardware, or include one of our supported board files to jumpstart the creativity, and hack on an existing piece of hardware.

If you are getting started, and have one of the Daisy Family Products, you can skip ahead to that section below.

## daisy.h

The base-level include file. This is all you need to include to create your own custom hardware that uses libdaisy.

`daisy_seed.h` is an example of a board level file that utilizes libdaisy to define some hardware, and provide flexible access.

## daisy_seed.h

The SOM-level include file. This can be used with any boards that use the Daisy Seed hardware.

Additional configuration files, with more specific hardware access are provided below for our supported hardware platforms.

## daisy_platform.h

Several other pairs of files exist in the repo for each of the supported hardware platforms that work with Daisy Seed.

These are:
- daisy_field
- daisy_patch
- daisy_petal
- daisy_pod

With these files a number of additional initialization, and configuration is done by the library.

This allows a user to jump right into their new product with a simple api to do things like:

```
#include "daisy_patch.h"

void callback(float *in, float *out, size_t size)
{
    if (patch_get_sw(SW_3) == TOGGLE_POS_UP)
    {
        val = patch_get_knob(KNOB_1);
        osc.freq = dsy_map(val, 0, 1, 20, 10000); 
    }
    else
    {
        val = 100; // steady tone;
        osc.freq = val;
    }
}
```

without having a full understanding of what's going on under the hood.

------

With this flexible approach to the hardware configuration, we hope to promote a lot of fantastic hardware along with code to go with it.

