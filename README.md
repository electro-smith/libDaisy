![](https://github.com/electro-smith/libdaisy/blob/master/resources/assets/banner.png)

[![Build Status](https://travis-ci.org/electro-smith/libDaisy.svg?branch=master)](https://travis-ci.org/electro-smith/libDaisy)

**libDaisy is a hardware abstraction library for the [Daisy Embedded Audio Platform.](https://www.electro-smith.com/daisy)** 

It  provides easy access to things such as GPIO, MIDI, USB communication, and more.

## Features

- Configurable Audio Callback

- MIDI Drivers 

- USB Communication (Audio, MIDI, Serial, etc.)

- Peripheral Device Drivers (SPI, I2S, I2C, etc.)

## Code Example
```c++
int main(void)
{
    // Init
    float samplerate;
    hw.Init();
    samplerate = hw.AudioSampleRate();
    midi.Init(MidiHandler::INPUT_MODE_UART1, MidiHandler::OUTPUT_MODE_NONE);

    midi.StartReceive();
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    for(;;)
    {
        midi.Listen();
        // Handle MIDI Events
        while (midi.HasEvents())
        {
            HandleMidiMessage(midi.PopEvent());
        }
    }
}
```

## Getting Started
- Check out our [Getting Started Wiki Page](https://github.com/electro-smith/DaisyWiki/wiki)
- Browse the reference documentation [on the web](https://electro-smith.github.io/libDaisy) or in [pdf form](https://github.com/electro-smith/libDaisy/blob/master/doc/libdaisy_reference.pdf)
- Make some sound!

## Project Overview

Prefixes and their meanings:

- **sys** - System level configuration (clocks, dma, etc.)
- **per** - Peripheral level, internal to MCU (i2c, spi, etc.)
- **dev** - External device support (external flash chips, DACs, codecs, etc.)
- **hid** - User level interface elements (encoders, switches, audio, etc.)
- **util** - library level elements used within the library (not included via daisy.h)
- **daisy** - core API files (specific boards and platforms have extended user APIs that configure libDaisy more below).

Also included is a core/ folder containing:

- a generic Makefile that can be included in a project Makefile to simplify getting started
- a linker script for defining the sections of memory used by the firmware
- core files for starting the hardware (system_stm32h7xx.c, startup_stm32h750xx.s, etc.)

### daisy.h

The base-level include file. This is all you need to include to create your own custom hardware that uses libDaisy.

`daisy_seed.h` is an example of a board level file that utilizes libDaisy to define some hardware, and provide flexible access.

### daisy_seed.h

The SOM-level include file. This can be used with any boards that use the Daisy Seed hardware.

Additional configuration files, with more specific hardware access are provided below for supported hardware platforms.

### daisy_platform.h

Several other pairs of files exist for each of the supported hardware platforms that work with Daisy Seed.

These are:
- daisy_field
- daisy_patch
- daisy_petal
- daisy_pod

With these files a number of additional initialization, and configuration is done by the library.

This allows a user to jump right in without needing a complete understanding of what's going on under the hood.

## Contributing

Here are some ways that you can get involved:
- Proof read the documentation and suggest improvements
- Test existing functionality and make [issues](https://github.com/electro-smith/libdaisy/issues) 
- Add new functionality to the library. See issues labeled "feature"
- Fix problems with existing codebase. See issues labeled "bug" and/or "polish"

Before working on code, please check out our [Contribution Guidelines](https://github.com/electro-smith/DaisyWiki/wiki/6.-Contribution-Guidelines) and [Style Guide.](https://github.com/electro-smith/DaisySP/blob/master/doc/style_guide.pdf)

## Support

Here are some ways to get support and connect with other users and developers:

- Join the [Daisy Forum](https://forum.electro-smith.com/)

- Make a [GitHub Issue](https://github.com/electro-smith/libdaisy/issues) 

- Join the [Daisy Slack Workspace](https://join.slack.com/t/es-daisy/shared_invite/zt-f9cfm1g4-DgdCok1h1Rj4fpX90~IOww)

## License
libDaisy is licensed with the permissive MIT open source license. 

This allows for modification and reuse in both commercial and personal projects. 
It does not provide a warranty of any kind. 

For the full license, read the [LICENSE](https://github.com/electro-smith/libdaisy/blob/master/LICENSE) file in the root directory. 
