# Getting Started - Serial Printing

One of the most effective ways of debugging your software without having to add additional hardware is to be able to print information from your source code.

In a normal C/C++ program you might use `printf` or `std::cout`, these can be set up to work on the Daisy to print over JTAG, but without any extra hardware we can log information out through the built-in USB port on the Daisy.

## Choosing a Serial Monitor

Your first question might be, "How can I see where it's printing on my computer?" And there are a ton of answers to this question.

The offerings for Serial Monitors vary from OS to OS, but there are a few options that are cross platform, and _just work_:

* Arduino - has a built in Serial Monitor that works great with Daisy.
* Serial Port Helper extension for VS Code is simple to use, and works great with Daisy.

(If you're using the Serial Port Helper extension you'll have to the "view" option from "hex" to "string" the first time you connect to the COM port.)

There are a number of other options out there, and if you've already used a different tool with success, feel free to keep using it!

## Hello World

A simple "Hello World", might look a little something like:

```cpp
#include "daisy_seed.h"

using namespace daisy;

DaisySeed hw;

int main(void) {

  // Initialize the Daisy Seed Hardware
  hw.Init();

  // Enable Logging, and set up the USB connection.
  hw.StartLog();

  // And Print Hello World!
  hw.PrintLine("Hello World!");

  while(1) {}
}
```

Chances are, if you tried to run this code, you might have missed the "Hello World" since it happens almost instantly after the program starts. It probably happened before you could connect to the Serial Port.

There are are a few ways to address this issue, and we'll look at both:

First, you could add a large delay between the Start Log, and the first thing you want to print:

```cpp
hw.StartLog();
System::Delay(5000);
hw.PrintLine("Hello World!");
```

This works, but you still only have 5 seconds to connect to the port before you might end up missing the message.

Another solution would be to move the PrintLine into the infinite while loop.

```cpp
. . .
hw.StartLog();
while(1) {
  hw.PrintLine("Hello World!");
  System::Delay(1000); // Wait 1 second between printing
}
```

This works, but if you only want to print something once, or you're debugging something in your initialziation code, then it's not very useful to continuously print it over and over again.

So what's the best solution? Well, there's an optional argument for the Logger that will wait for connection to the COM port before resuming code execution.

By calling:

```cpp
hw.StartLog(true);
```

The program will wait indefinitely until you connect before moving on.

So, for our best Hello World, where it's impossible to miss the message on the Serial Monitor:

```cpp
#include "daisy_seed.h"

using namespace daisy;

DaisySeed hw;

int main(void) {

  // Initialize the Daisy Seed Hardware
  hw.Init();

  // Enable Logging, and set up the USB connection.
  // Setting true here means that the program will wait until
  // a connection has been made to a USB Host
  hw.StartLog(true);

  // And Print Hello World!
  hw.PrintLine("Hello World!");

  while(1) {}
}
```

Now, there are times where this is exactly what we want, and there's times where we want our board to run with, or without the USB connected. In the latter case, we will instead choose one of the other potential solutions, or create special cases for DEBUG builds vs RELEASE builds.

## String Formatting

For those of you used to format specifiers from C or C++, you will feel right at home as the `Print` and `PrintLine` functions both use the same syntax.

For those unfamiliar, you can find information on the specifiers in the [cpp language documentation](https://en.cppreference.com/w/cpp/io/c/fprintf).

For example, consider counting something:

```cpp
. . .
// Declare an int set to 0
int mycnt = 0;
while(1) {
  // Print the value:
  hw.PrintLine("Count: %d", mycnt);
  // And then increment the counter
  mycnt++;
  // If the counter exceeds 100, reset it to 0
  if (mycnt > 100)
    mycnt = 0;
  // And count slow enough that it's easy to watch on the Serial Monitor
  System::Delay(250);
}
```

This example will output:

```shell
Count: 0
Count: 1
Count: 2
```

and so on all the way up to 100 before repeating itself.

## Floating Point Numbers

Due to the small memory footprint of an embedded platform like daisy, floating point numbers are handled slightly differently by default, and won't work with the `%f` format specifier.

Each of the following options has its own benefits:

* Adding the `LDFLAG` for `%f` support: For ease-of-use, when memory limits are not an issue.
* Using the `FLT_FMT`, `FLT_VAR` specifiers: For minimal cost, drop in for printing the ocassional float.
* FixedCapStr: When you know the size of the strings, building UIs, etc.

### Using %f specifier

That said, if you really want to be able to use the `%f`, and your application isn't nearing the maximum size, you can add the following to your project's application Makefile:

```Make
LDFLAGS += -u _printf_float
```

This does add a few kilobytes to the flash usage of any program, and is why it's not included by default.

With that flag added you can print floats as you would expect:

```cpp
float my_flt = 123.456f;
hw.PrintLine("My Float: %f", my_flt);
```

### Using FLT_FMT and FLT_VAR macros

Without the extra memory cost of adding that linker flag, there are a few other ways to output floating point numbers:

One is to use special macros provided with the Logger class:

```cpp
float my_flt = 123.456f;
hw.PrintLine("My Float: " FLT_FMT3, FLT_VAR3(my_flt));
```

To specify more or less decimal places you can use the more generic macros

```cpp
float more_precice_float = 123.456789f;
hw.PrintLine("My Float: " FLT_FMT(6), FLT_VAR(6, my_flt));
```

### Using FixedCapStr class

Another way is to use the `FixedCapStr` class, and the `appendFloat` function:

```cpp
FixedCapStr<16> str("Value: ");
str.AppendFloat(123.456f);
hw.PrintLine(str);
```

You'll notice that the default for the `AppendFloat` function is to round to the nearest two decimal places. You can adjust this to print more decimal places by adding a second argument:

```cpp
str.AppendFloat(123.456f, 3);
```

This last option has a lot of flexibility for other types, and works well when building complex user interfaces with Displays, and the `Ui` class. You do have to be mindful that the template argument is large enough for the final string you want to make.

## Further Reading

Topics without links are coming soon:

* Reading and Writing to SD Cards and USB Drives
* [Working with ADCs](_a4_Getting-Started-ADCs.md)
* Building UIs for Displays
