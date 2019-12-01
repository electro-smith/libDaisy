
# svf
     Double Sampled, Stable State Variable Filter

Credit to Andrew Simper from musicdsp.org

This is his "State Variable Filter (Double Sampled, Stable)"

Additional thanks to Laurent de Soras for stability limit, and 
Stefan Diedrichsen for the correct notch output

Ported by: Stephen Hensley

example:
daisysp/examples/svf/

### init

Initializes the filter

float samplerate - sample rate of the audio engine being run, and the frequency that the process function will be called.
```c
            void init(float samplerate);
```

### process

Process the input signal, updating all of the outputs.
```c
            void process(float in);
```

## Setters


### set_freq

sets the frequency of the cutoff frequency. 

f must be between 0.0 and samplerate / 2
```c
            void set_freq(float f);
```

### set_res

sets the resonance of the filter.

Must be between 0.0 and 1.0 to ensure stability.
```c
            void set_res(float r);
```

### set_drive

sets the drive of the filter, affecting the response of the resonance of
the filter..
```c
            inline void set_drive(float d) { _drive = d; }
```

## Filter Outputs

## Lowpass Filter
```c
            inline float low() { return _out_low; }
```

## Highpass Filter
```c
            inline float high() { return _out_high; }
```

## Bandpass Filter
```c
            inline float band() { return _out_band; }
```

## Notch Filter
```c
            inline float notch() { return _out_notch; }
```

## Peak Filter
```c
            inline float peak() { return _out_peak; }
```
