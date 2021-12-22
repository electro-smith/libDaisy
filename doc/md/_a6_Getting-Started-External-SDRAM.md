# Getting Started - External SDRAM

## Usage

The Daisy comes with 64MB of external memory in addition to it's 1MB of internal memory

This is extremely valuable for DSP effects like delays, sample-based synthesis, and more.

The external memory, however, does require a little bit of special handling to use.

Let's first look at a normal array of floats. Something we might use for a buffer or delay:

```cpp
float my_buffer[1024];
```

So that's 1024 floats in an array within our normal memory. Totalling 4kB of memory. This will easily fit in any of the memory regions available on the Daisy.

You can declare that anywhere and do anything with it.

Well, the SDRAM is a little bit different.

Due to the way the SDRAM is handled, an object located within it has to be created globally (at least the easy way), and it can't have a constructor of any importance (a C++ construct that happens before the SDRAM is fuly initialized).

Now, for most things (like buffers, arrays, etc.) this doesn't matter much.

So, let's make the same array again, but in the SDRAM:

```cpp
float __attribute__(section((".sdram_bss"))) my_buffer[1024];
```

Now, that is a lot of stuff to type, and pretty hard to remember. So we made a shorthand macro to make it a bit easier:

```cpp
float DSY_SDRAM_BSS my_buffer[1024];
```

The first, longform example is the spelled out compiler attribute that says what memory section to put the array in. This can be useful on big projects when using one of the many other available memory sections within the STM32H7 processor.

However, for now, we don't need to worry much about that, and can just use the `DSY_SDRAM_BSS` macro.

Now, when you use this memory it's initial condition is undefined, meaning it could be zeroes, or it could be whatever it was last if you only turned the board off for a few milliseconds, or did a software RESET.

Beyond that, this memory operates in much the same way as normal memory, just a little bit slower. For most things this won't be noticeable, but if you're trying to do extreme-granular-wavetable super-synthesis, you may have to pay attention to what memory you're using.

## Addressing

The SDRAM is memory-mapped to `0xC0000000`. So if you're using pointers, or want to create some sort of dynamic allocator of some kind, that would be the base address of the SDRAM. It is totally usable across it's range. Just keep in mind some of the limitations mentioned above.

## Initialization

Most objects that might expect SDRAM memory within libDaisy, or more-often DaisySP, use `Init` functions meant to be called after the `DaisySeed::Init` to fill the memory with expected data. If you're building your own looper, sampler, delay, etc. you'll want to make sure to zero, or fill the SDRAM allocated buffer _after_ within a function that can be called from the Init function instead of during a constructor.

## Class Design with Large Memory

The simplest way to allow for a huge chunk of memory to be use an external buffer type of some sort.

The most rudimentary method of doing so would look something like:

```cpp
MyClass::Init(float* buffer, size_t size);

. . .
float *internal_buffer_;
```

Where a float buffer can be declared of any size, in any memory region, and passed in at init-time.

## Future Work

There are plans to initialize the SDRAM during startup to allow access via constructors, or as the default heap, etc.

So these limitations are not set in stone. Check in the [Release Notes](https://github.com/electro-smith/libDaisy/releases), and the [Forum](https://forum.electro-smith.com) for updates in the future.
