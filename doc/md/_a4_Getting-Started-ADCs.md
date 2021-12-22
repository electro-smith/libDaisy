# Getting Started - ADCs

ADC stands for Analog to Digital Converter. These are used to read in a variable signal. These variable signals can be anything from a potentiometer, photoresistor, external control voltage, etc.

On the Daisy, the ADC inputs expect a signal in the range of 0V to 3.3V, unless otherwise stated (e.g. The CV inputs on the Daisy Patch SM are set up to expect -5V to 5V, a common range for Modular Synthesizers).

In this tutorial we'll be using the Daisy Seed hardware, and we will go through the process of connecting a few inputs to one of the many available ADC pins.

## The CPP Objects Used

We'll be using the `DaisySeed`, it's internal `AdcHandle`, `Pin`, and `AdcChannelConfig` objects in the following sections.

If you're using a different Daisy SOM (i.e. DaisyPatchSM, etc.) this is handled within the board support files for all available inputs.

The `DaisySeed` object is a class that manages all of the hardware on the Seed board. All we need to do with it for now is initialize it.

The `AdcHandle` is a handler for the microcontroller's embedded Analog to Digital Converter. It is up to 16-bits, and can handle multiplexing several inputs.

The `Pin` class is used to describe a specific physical pin on the hardware. These objects are used to initialize GPIO, but are also used in the configuration of more complex peripherals and devices (i.e. ADCs, Shift Registers, etc.).

The `AdcChannelConfig` object is used to define each of the connections to the ADC, and the order in which the inputs will be read.

## The Hardware Connections

For this tutorial all we'll need is:

* a few potentiometers like these [9mm Plastic Shaft Pot with Pointers](https://www.electro-smith.com/parts/trimmer), but any potentiometer should do the trick.

To wire up the pot we'll want to connect pin 1 of the pot to GND, and pin 3 of the pot to 3v3_A, and the middle leg (pin 2) will now output a variable voltage from 0V to 3.3V; perfect for our expected input range.

When working with the Daisy and external hardware, **always connect the DGND and AGND pins outside of the Daisy**

## ADC Channel Config

libDaisy uses the STM32 ADC1 peripheral, which is a single ADC with several multiplexed inputs.

This makes setting it up a little different than something like a GPIO, because the ADC will scan through all of the configured inputs in the background without taking up CPU time.

To define what connections are made we will use the `AdcChannelConfig` class.

The AdcChannelConfig has two possible initialization functions:

* `InitSingle` - For configuring a single input to a single pin on the Daisy
* `InitMux` - For configuring several inputs from an external multiplexer (like CD4051) to a single pin on the Daisy

Below we'll only be using the `InitSingle` function, but stay tuned for a guide for connecting a ton of controls using external multiplexer in the future.

The `InitSingle` function takes an old version of the Daisy's `Pin` object, but it is compatible with the new pins. So anywhere you see, `dsy_gpio_pin` you can use the `Pin` object instead.

This structure, and the number of channels can then be passed to the `AdcHandle` to set everything up.

## A Single Input

Alright, enough talk. Let's plug some stuff in, and write some code!

We can take a look at all of the available ADC pins by looking at the [Pinout Diagram](https://github.com/electro-smith/DaisyWiki/wiki/2.-Daisy-Seed-Pinout). The ADC Pins are all labeled and colored in yellow.

Let's get started with pin `A0` (also known as `D15`), and wire up our potentiometer.

See the wiring mentioned above in the [Hardware Connections](#The-Hardware-Connections) for how to connect all of the legs of the pot, and take the wiper (middle leg), and connect that to the `A0` pin on the Daisy.

And here's a short program that will setup, and read from an ADC:

```cpp
#include "daisy_seed.h"

using namespace daisy;
using namespace daisy::seed;

// Create out Daisy Seed Hardware object
DaisySeed hw;

int main(void) {
  // Initialize the Daisy Seed hardware
  hw.Init();

  // Start logging for printing over serial
  hw.StartLog();

  // Create an ADC Channel Config object
  AdcChannelConfig adc_config;

  // Set up the ADC config with a connection to pin A0
  adc_config.InitSingle(A0);

  // Initialize the ADC peripheral with that configuration
  hw.adc.Init(&adc_config, 1);

  // Start the ADC
  hw.adc.Start();

  while(1) {
    // Read the first ADC that's configured. In our case, this is the only input.
    int value = hw.adc.Get(0);

    // In order to know that everything's working let's print that to a serial console:
    hw.PrintLine("ADC Value: %d", value);

    // Wait half a second (500 milliseconds)
    System::Delay(500);
  }
}
```

To find out more about the serial printing, and what software to use, see our [tutorial on working with Serial Printing](_a2_Getting-Started-Serial-Printing.md).

With the above program, you should see a number printing that moves from 0 to 65536 as you rotate the pot. Now, pots, and electronics aren't perfect.
So there's a chance that this may not reach those extremes, or it might reach them slightly before the edges of the potentiometer's range.
This will largely depend on the potentiometer you're using.

Now that's great and all, but it would be nicer to work with a number that is scaled between 0 and 1 instead (since that's more useful for doing other things).

We can actually get the value in that range instead by changing the line where we were doing:

```cpp
int value = hw.adc.Get(0); // value will be 0-65536
```

to this Instead:

```cpp
float value = hw.adc.GetFloat(0); // value will be 0.0 to 1.0
```

floating point numbers have a few peculiarities because of the hardware and memory requirements of an embedded project, but you can keep the serial printing by adding the following to the project Makefile:

```make
LDFLAGS += -u _printf_float
```

This isn't included by default because it increases the flash size by several kilobytes, and there are other ways to print floating point numbers (more info on this in the [serial printing tutorial](tutorial)).

## Multiple Inputs

Most things get better when you add more of whatever it is. The same goes for controls!

Now we've set up a single pot. So let's look at what it would look like to hook up two (or more).

Nearly everything will stay the same except instead of using a single `AdcChannelConfig`, we'll want to create one for each control we want to use. C and C++ have a mechanism for dealing with this known as an array. If you're not familiar, it is basically just an ordered list.

So for setting up two pots we'll change the initialization code to look a little more like:

```cpp
// Create an array of two AdcChannelConfig objects
const int num_adc_channels = 2;
AdcChannelConfig my_adc_config[num_adc_channels];
// Initialize the first one connected to A0
my_adc_config[0].InitSingle(A0);
// Initialize the second one connected to A4
my_adc_config[1].InitSingle(A4);
```

And finally when we go to initialize the ADC itself, we can now omit the `&` that we had to use before because it's an array of objects, and we want to make sure to update the total count that we passed in before:

```cpp
hw.adc.Init(my_adc_config, num_adc_channels);
```

Now you can read from either input. If you want to read the pot connected to A0 you would use `hw.adc.Get(0)`, or `hw.adc.GetFloat(0)`, and if you want to read from the pot connected to A4, you would use `hw.adc.Get(1)`, or `hw.adc.GetFloat(1)`.

You'll notice that you access them by the order in which you initialized them, not by the number associated with the Pin.

## Order To Chaos - Naming ADC Channels

Once you have a lot of ADC channels, it can be difficult to remember which channel corresponds to which control. To solve this, you can use `enum`s to assign names to channels like this:

```cpp
enum AdcChannel {
   pitchKnob = 0,
   pitchCv,
   gainKnob,
   gainCv,
   NUM_ADC_CHANNELS
}
```

Here, the first value is assigned to `0`and the others increment automatically. Interestingly, this logic can be used to automatically get the total number of channels you use, by adding one last value `NUM_ADC_CHANNELS`. Of course you could also specify the individual values by hand, just add `myKnob = 3,`.

Now you can make your code much easier to read - and maintain, should you decide to change the hardware connections later:

```cpp
AdcChannelConfig my_adc_config[NUM_ADC_CHANNELS];
// ...
my_adc_config[pitchCv].InitSingle(A0);
// ...
hw.adc.Init(my_adc_config, NUM_ADC_CHANNELS);
// ...
const float pitchCvValue = hw.adc.GetFloat(pitchCv);
```

## Using Different Kinds of Inputs

OK. So now we know what to do with one, or a few inputs that already conform to the expected input range.

In a future guide we'll look at using active and passive circuits to condition a signal to the best range for sending to the Daisy.

## Further Reading

Topics coming soon:

* Printing with USB
* HID classes (Switch, Led, AnalogControl) and why we use them
