# Getting Started - Audio

Audio is an essential part of the daisy ecosystem, but isn't entirely simple to work with if you're not used to writing code in general.

Let's breakdown how the audio on an embedded device like Daisy works, and go over how can start making sound immediatly with your daisy.

## Analog and Digital Audio

What is Analog Audio, and how is different from "digital audio"? These are just a few questions you may have wondered at one point or another.

Analog audio, as it pertains to Daisy, is a voltage that that goes into the audio input pins, or leaves the audio output pins. This is typically a line level signal, or has an accompanying circuit to convert it from one signal level to another (i.e. Modular level on the Daisy Patch, etc.).

Digital Audio is the representation of the signal while it's inside one of the devices on the daisy, in your computer, or stored on a drive somewhere in your room or office. Digital Audio can be represented a number of different ways, and is often discussed using the terms "sample rate", and "bit depth".

### Sample Rate

Sample rate is the number of samples processed each second. Typical CDs are at 44.1kHz, while movies and other media have audio at 48kHz. That means that there are roughly 45-thousand discrete audio samples, or individual values, for each second of audio recorded.

High-fidelity samplerates start at 96kHz, and 192kHz, and can go even higher in measurement devices! The daisy's on board audio can run up to 96kHz.

### Bit Depth

Meanwhile, bit-depth is the term used to describe each of those individual values. Remember the 48000 samples per second? Well, what are they; Letters, numbers, and if so, what _kind_ of number?

Well, typical CD audio is 16-bits, that means that for any single value there are 65536 potential "volume" levels that can be used to represent the signal. On the daisy, the hardware uses 24-bit audio which comes out to over 16.7 **million** values for a given sample. 

On the other side of the spectrum, "Lo-Fi" audio would be something like 8-bit audio (found in early video game consoles, and elsewhere). In this case, there are only 256 potential values for a given sample. 

The higher the bitdepth, the more dynamic range, and the less "digital" something may sound... to a point. (beyond 16-bits, the difference becomes less and less noticeable).

The bitdepths mentioned so far are known as "integer" formats, meaning that the values are represented as whole numbers. For example, a 16-bit sample would be a number between -32768 and 32767.

32-bit float bitdepth represents the numbers as floating point numbers. Daisy's audio engine uses 32-bit floats to work with audio, and converts to whatever the hardware connected to it wants. That said, by using only the `SaiHandle` without the `AudioHandle` it is possible to work with audio in the "fixed-point" formats mentioned above.

### Block Size

Digital audio is often handled in blocks, instead of acquiring, processing, and delivering one sample at a time. It is more efficient to do this, as the hardware takes a little bit of extra time to set up the transaction to acquire, and deliver new samples. When working with blocks, you can be filling a new group of samples while the last group is being sent through the hardware. 

This is set up in side of the `AudioHandle` class, and doesn't require any special attention, but it is worth being aware of.

When working with extrememly large blocks, this can introduce some noticeable latency. However, with the sizes common to Daisy, the software latency this adds is usually around, or less than 1 millisecond (i.e. imperceivable).

For our examples below, we'll be using a very small block size of 4 samples so that we can visualize the data structures easily.

## Making Some Sound

If you're using the Daisy Seed, DaisyPatchSM, or any of the other boards that build off of one of those, the audio engine is set up under the hood, and all you have to do to start making some sound is create a callback.

So what is a callback? Well, a callback is a piece of code that will be "call"ed whenever a certain event happens. In the case of Daisy Audio, this is whenever the hardware is ready for more samples (those digital bits of audio we talked about earlier).

The daisy audio callback usually looks something like this:

```cpp
#include "daisy_seed.h"
using namespace daisy; // to simplify syntax

// Create our hardware object
DaisySeed hw;

void MyCallback(AudioHandle::InputBuffer in, 
                AudioHandle::OutputBuffer out, 
                size_t size) 
{
    // Make sound here!
}

int main(void)
{
    hw.Init();
    hw.StartAudio(MyCallback);
    while(1) {}
}
```

The above example shows starting the audio engine, with our callback, but doesn't actually do anything with the sound coming in, or create any sound to go out.

### Audio Output

Where would we start if we wanted to make some sound? Well, there are two things we need to know if we want to _output_ some sound.

1. What is "out", What is a AudioHandle::OutputBuffer?
2. What does size mean?

Well, "out" are the samples that we can use to make sound, and "size" is the number of samples we have to fill. In other words, its our block size.

The out buffer is "non-interleaved" meaning that there are two "rows" of data, one for the left audio channel, and one for the right (or in the case of the Daisy Patch, there are four rows, one for each channel of audio).

If we want to write silence to all of the audio it would look something like:

```cpp
void MyCallback(AudioHandle::InputBuffer in, 
                AudioHandle::OutputBuffer out, 
                size_t size) 
{
    for (size_t i = 0; i < size; i++)
    {
        out[0][i] = 0.0f;
        out[1][i] = 0.0f;
    }
    // you can also use standard library functions if you're familiar with C++:
    std::fill(&out[0][0], &out[0][size], 0.0f);
}
```

Where the first bracket indicates the audio channel (0 for left, 1 for right).

We can loop through each sample, and choose what value it should be.

Since silence is boring, and creating an oscillator is a bit out of the scope of this guide, the [DaisySP](https://github.com/electro-smith/DaisySP) DSP Library is full of objects that you can use to do things instead of silence, without having to know exactly what you're doing, or spending time writing DSP yourself.

For example, we can use the `daisysp::Oscillator` class to generate a tone. The defaults for Oscillator will be acceptable for this (it will generate a 100Hz sine wave). So we won't go in depth on what else this class can do right now.

```cpp
#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;

DaisySeed hw;
daisysp::Oscillator osc;

void MyCallback(AudioHandle::InputBuffer in, 
                AudioHandle::OutputBuffer out, 
                size_t size) 
{
    for (size_t i = 0; i < size; i++)
    {
        // The oscillator's Process function synthesizes, and
        // returns the next sample.
        float sine_signal = osc.Process();
        out[0][i] = sine_signal;
        out[1][i] = sine_signal;
    }
}

int main(void)
{
    hw.Init();
    // We initialize the oscillator with the sample rate of the hardware
    // this ensures that the frequency of the Oscillator will be accurate.
    osc.Init(hw.AudioSampleRate());
    hw.StartAudio(MyCallback);
    while(1) {}
}
```

And with that, we have sound!

Now, working with the input isn't too much different.

### Audio Input

The input comes in the same way that the audio goes out, as arrays of floating point values.

If we simply want to pass audio through the Daisy without doing anything we just need to copy the data from the input buffer to the output buffer.

```cpp
for (size_t i = 0; i < size; i++)
{
    out[0][i] = in[0][i];
    out[1][i] = in[1][i];
}
```

but what's the point in setting up all of this code to make something sound the same, let's use the oscillator we already added to make something interesting happen:

```cpp
for (size_t i = 0; i < size; i++)
{
    float sine_signal = osc.Process();
    out[0][i] = in[0][i] * sine_signal;
    out[1][i] = in[1][i] * sine_signal;
}
```

Now, whatever you send through the daisy is going to get multiplied by the 100Hz sine signal. Sounds pretty funky, right!? Well, that's a type of amplitude modulation known as ring modulation, which is a topic for another day, but it goes to show that with very little code, and some curiousity we can start to make some interesting things happen.

[DaisySP](https://github.com/electro-smith/DaisySP) is filled with other objects to filter, delay, bit crush, reverberate, and otherwise synthesize, and process sound, and can be a great stepping off point from here to making some interesting sounds.

### Audio Data Packing

On the daisy, there are two ways the audio data can be prepeared, "Interleaved" and "Non-Interleaved", the overall callback structure functions the same in either case, but the way the input and output data is packed is different in each one.

Above, we looked at the non-interleaved format, where each channel had its own array of samples.

Interleaved audio is instead one large array containing multiple channels of data. This data is interleaved so that there is a single sample of each channel before moving to the next sample. For example, with a block size of four, the full buffer would look like:

```
{ L0, R0, L1, R1, L2, R2, L3, R3 }
```

as opposed to the non-interleaved version of the same data, that could be represented as:

```
{ 
    { L0, L1, L2, L3 },
    { R0, R1, R2, R3 }
}
```

## Changing Audio Settings

The AudioHandle allows the user to change certain settings, in particular the:

* Sample Rate
* Block Size

On the DaisySeed, or other official boards you can use the following functions:
```cpp
hw.SetAudioBlockSize(4);
hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
```

Block size can be any number up to 256.

See the [SaiHandle::Config::SampleRate](@ref daisy::SaiHandle::Config::SampleRate) enum for the available samplerate options. It's also worth mentioning that, depending on what codec is connected, not all options are compatible.

## What Not to do in the Audio Callback

The audio callback is in what's called and "Interrupt Handler" without going in depth on what that means here, it does mean that it the function _must_ take less time than it takes to transfer out the audio buffer, otherwise your audio will start to have under-run errors, which cause digital artifacts in the audio path.

The amount of time can be calculated as `(1 / sample_rate) * block_size`. For example, with 48 sample block size at 48kHz, the function can take 1ms before underruns start to occur.

Now, that is a bit of a simplification, because chances are you'll be doing a few things in the main `while()` loop, but it does mean that you shouldn't do things that take an indeterminant amount of time.

These things include:

* calls to other peripheral blocking functions (i.e. DAC Write, SPI tranfer, etc.)
* dynamic allocation like `malloc`

If you do need to trigger non-deterministic events from within the audio callback you can use a flag, or other mechanism that can be checked in the main loop. An overly simplified example of this might look like:

```cpp
// Global:
bool action_flag;

// In callback:
if (some_event) // like a button press, or something
    action_flag = true; 

// In main()

while(1) 
{
    
    if (action_flag) {
        // Do the big thing that can take a while
        initiate_big_transfer();
        // and clear the flag
        action_flag = false;
    }
}
```

## How To Measure the Processing Load

It is crucial to have a feeling of how much processing load your algorithm introduces and how much room for additions you still have. This can be done by measuring the execution time of the audio callback and comparing it to the available time. Daisy comes with a helper class to do this, the `CpuLoadMeter`.

To measure the CPU load, you would call `.OnBlockStart()` at the beginning, and `.OnBlockEnd()` at the end of your audio callback. The you can request the resulting load measurement and print it on a display or a serial connection. It is crucial to do the printing from the main loop, not the audio callback itself, as discussed before.

A complete example would look like this:

```cpp
#include "daisy_seed.h"
#include "daisysp.h"
using namespace daisy;
DaisySeed hw;
CpuLoadMeter loadMeter;

void MyCallback(AudioHandle::InputBuffer in, 
                AudioHandle::OutputBuffer out, 
                size_t size) 
{
    loadMeter.OnBlockStart();
    for (size_t i = 0; i < size; i++)
    {
        // add your processing here
        out[0][i] = 0.0f;
        out[1][i] = 0.0f;
    }
    loadMeter.OnBlockEnd();
}
int main(void)
{
    hw.Init();
    
    // start logging to the serial connection
    hw.StartLog();
    
    // initialize the load meter so that it knows what time is available for the processing:
    loadMeter.Init(hw.AudioSampleRate(), hw.AudioBlockSize());
    
    // start the audio processing callback
    hw.StartAudio(MyCallback);
    
    while(1) {
        // get the current load (smoothed value and peak values)
        const float avgLoad = cpuLoadMeter.GetAvgCpuLoad();
        const float maxLoad = cpuLoadMeter.GetMaxCpuLoad();
        const float minLoad = cpuLoadMeter.GetMinCpuLoad();
        // print it to the serial connection (as percentages)
        hw.PrintLine("Processing Load %:");
        hw.PrintLine("Max: " FLT_FMT3, FLT_VAR3(maxLoad * 100.0f));
        hw.PrintLine("Avg: " FLT_FMT3, FLT_VAR3(avgLoad * 100.0f));
        hw.PrintLine("Min: " FLT_FMT3, FLT_VAR3(minLoad * 100.0f));
        // don't spam the serial connection too much
        System::Delay(500);
    }
}
```
