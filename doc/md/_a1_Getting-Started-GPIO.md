# Getting Started - GPIO

GPIO stands for General Purpose Input/Output. These are common to microcontrollers of all sizes, and are one of the many ways to work with external components.

GPIO are used as the basis for several interface components including switches, LEDs, encoders, etc.

The Daisy exposes several GPIO on its pinout. Most of these pins can do much more than just be a digital input or output, but we'll keep it simple for now.

Below, we'll discuss the code, hardware, and provide a few examples of how to use the GPIO within the Daisy ecosystem.

## The CPP Objects Used

We'll be using the `DaisySeed`, `Pin`, and `GPIO` objects in the following sections.

If you're using a different Daisy SOM (i.e. DaisyPatchSM, etc.) this still applies, but some of the pin names will be different.

The `DaisySeed` object is a class that manages all of the hardware on the Seed board. All we need to do with it for now is initialize it.

The `Pin` class is used to describe a specific physical pin on the hardware. These objects are used to initialize GPIO, but are also used in the configuration of more complex peripherals and devices (i.e. ADCs, Shift Registers, etc.).

The `GPIO` class is used for the basic reading/writing of a digital signal.

When we talk about "digital" signals, we mean a signal that has only two states, HIGH and LOW.

## The Hardware Connections

For the examples below we'll be using a few components. These can be replaced with a number of similar alternatives.

* [Mini Pushbutton](https://www.electro-smith.com/parts/pushbuttons)
* [3mm LED](https://www.electro-smith.com/parts/3mm-leds)
* Resistor(s) any low value (1-10K should work okay in the examples discussed below)

## GPIO Input

One of the simplest uses of a GPIO is to read the state of a button.

For this example we'll use a simple [mini pushbutton](https://www.electro-smith.com/parts/pushbuttons) to read the state of a button.

The easiest way to wire up a switch like this is to connect one set of legs (pins 1 and 2) to the GPIO input, and connect the other two legs (pins 3 and 4) to GND.

A very short example of a program to do this looks like:

```cpp
#include "daisy_seed.h"

using namespace daisy;
using namespace daisy::seed;

DaisySeed hw;

int main(void) {
  // Initialize the Daisy Seed
  hw.Init();

  // Create a GPIO object
  GPIO my_button;

  // Initialize the GPIO object
  my_button.Init(D0, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

  while(1) {
    // And let's store the state of the button in a variable called "button_state"
    bool button_state = my_button.Read();
  }
}
```

If you haven't followed the guides on [setting up your development environment](https://github.com/electro-smith/DaisyWiki/wiki/1.-Setting-Up-Your-Development-Environment) and [creating a new project](https://github.com/electro-smith/DaisyWiki/wiki/How-To:-Create-a-New-Project) now would be a perfect time to catch up, and create a new project to work with the GPIO.

Compiling this example, you might get a warning, for an "unused variable button_state", but we'll get to using that shortly!

## Breaking Down the Init function

Let's look at that GPIO init function in a bit more detail, shall we? First, we'll breakdown the full Initialization definition:

```cpp
void Init(Pin p, Mode m=Mode::INPUT, Pull pu=Pull::NOPULL, Speed sp=Speed::LOW);
```

The first thing you may be wondering, if you're not used to C++ is the `::` characters all over the place. These are used for the configuration settings so that the compiler will only allow you to use the correct inputs instead of being able to pass the wrong thing to the input, or even just mis-order the arguments, and end up scratching your head for hours wondering what's not working.

These `::` are also used for namespaces, which we handle in the above example with the `using namespace` lines. Without that we'd have to use `daisy::GPIO` instead of `GPIO`, `daisy::seed::D0` instead of `D0`, etc.

### Pin Argument

So, the first argument: `Pin p` describes the daisy pin to assign to our GPIO, this ties our new GPIO object to some of the actual hardware on the Daisy Seed. By using `D0` (or more explicitly, `daisy::seed::D0`), we're saying we want to use that pin for our GPIO.

### Mode Argument

The second argument: `Mode m` can be any of the following:

* `Mode::INPUT` - configures the pin as an input
* `Mode::OUTPUT` - configures the pin as an output (in push pull configuration)
* `Mode::OUTPUT_OD` - also an output, but the transistor connection to pull the signal to GND is not connected, this is less commonly used.
* `Mode::ANALOG` - configures the GPIO for connection to the ADC or DAC peripherals within the micro controller.

If you only supply the `Pin` argument, the GPIO will default to `Mode::INPUT` configuration.

### Pull Argument

The third argument: `Pull pu` is used to select whether the GPIO will use an internal (around 30-50k) resistor as a pull up or pull down resistor.

A pull up resistor will keep the GPIO idling at 3V3 unless something pulls it down, while a pull down resistor will keep a GPIO idling at 0V unless something pulls it up.

The options for this argument are:

* `Pull::NOPULL` - no pull up or pull down resistor is connected.
* `Pull::PULLUP` - pull up resistor is connected to the GPIO line.
* `Pull::PULLDOWN` - pull down resistor is connected to the GPIO line.

This argument will default to `Pull::NOPULL` if you only supply a `Pin` and a `Mode`

So when, and why do we need pull up or pull down resistors anyway?

Well, if we connect a wire to one of Daisy's GPIO pins, and there are no internal or external resistors attached, there's no definite way to know what voltage the wire is sitting at.

Now, if we connect the wire to a button, as we did in the above example, and connect the other side of the button to 3V3 then we _do_ know the voltage at the pin when the button pressed.
However, when the button is not pressed, we still have an "undefined" state, as we can't guarantee that the pin will be at 0V when the button isn't pressed.

To solve this unknown state issue, we can add a resistor to the pin that "pulls" the signal to GND when the button isn't actively pulling the voltage up to 3V3 (while pressed).

Instead of adding an external resistor to do this, we can use the built-in pull-down feature to accomplish the same thing.

In the example above, we did the same thing except we used the pull up resistor, and wired the button to GND. This usually just requires less wires, but the function is the same.

### Speed Argument

The final argument, `Speed sp` is a special configuration for Output modes that control the slew rate, or the speed at which it can switch from 0V to 3V. We won't go super in depth on that in this, but you'll always want it to be the lowest speed possible for a given application to reduce the amount of electrical noise emitted.

## GPIO Output

So you've got your button reading code, your board wired up, and you're probably thinking, "Great! So how can I tell if I've done this correctly!?".

Well, let's set up a second pin as an output to light up an LED when we push our button. That way we can tell everything we've done so far is working.

Setting up the GPIO output isn't much more work.

On your hardware, you'll want to take the second pin (right above the square D0 pin), and connect a resistor, and an LED in series. You'll want the **cathode** of the LED connected to GND, with the **anode** connected to the resistor, [as described here](https://makecode.adafruit.com/learnsystem/pins-tutorial/devices/led-connections).

Once you've got that set up we can add a few lines to our example.

In the Initialization section:

```cpp
GPIO my_led;

my_led.Init(D1, GPIO::Mode::OUTPUT);
```

And then we can use the `button_state` variable we were reading from the button earlier to control the state of the LED:

```cpp
while(1)
{
  // And let's store the state of the button in a variable called "button_state"
  bool button_state = my_button.Read();
  my_led.Write(button_state);
}
```

And that's it! Easy, right?

So you get that all set up, and you might be thinking, "Well, this is great, but the LED turns off when I push the button instead of turning it on, what the heck?"

That's because of the way we set up the button in the first step. Because we used the internal pull up resistor, to avoid adding extra parts to our board, the GPIO is returning true while the button is not pressed instead of what we might expect. Well, C++ has a very easy way of flipping that over.

```cpp
bool button_state = my_button.Read(); // state is false while button is pressed
bool button_pressed = !button_state; // "!" is a "not" operator, which will flip a bool var.
```

So all together, our short little light switch program will look something like:

```cpp
#include "daisy_seed.h"

using namespace daisy;
using namespace daisy::seed;

DaisySeed hw;

int main(void) {
  // Initialize the Daisy Seed
  hw.Init();

  // Create our GPIO object
  GPIO my_button;
  GPIO my_led;

  // Initialize the GPIO object for our button */
  my_button.Init(D0, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

  // Initialize the GPIO object for our LED
  my_led.Init(D1, GPIO::Mode::OUTPUT);

  while(1) {
    // And let's store the state of the button in a variable called "button_state"
    bool button_state = my_button.Read();

    // And we want to light up the LED while we're pressing the button 
    // so let's use the "!" to flip over the button_state
    my_led.Write(!button_state);
  }
}
```

## Further Reading

Topics coming soon:

* Working with ADCs
* Serial Printing over USB
* Using the Switch and LED classes instead of GPIO, and Why?
