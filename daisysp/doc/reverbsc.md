
# reverbsc

    Stereo Reverb

Ported from soundpipe

example:

daisysp/modules/examples/ex_reverbsc


### init
Initializes the reverb module, and sets the samplerate at which the process function will be called.
```c
    void init(float samplerate);
```

### process
process the input through the reverb, and updates values of out1, and out2 with the new processed signal.
```c
    void process(float in1, float in2, float *out1, float *out2);
```

### set_feedabck
controls the reverb time. reverb tail becomes infinite when set to 1.0

range: 0.0 to 1.0

```c
    inline void set_feedback(float fb) { _feedback = fb; }
```

### set_lpfreq
controls the internal dampening filter's cutoff frequency.

range: 0.0 to samplerate / 2

```c
    inline void set_lpfreq(float freq) { _lpfreq = freq; }
```
