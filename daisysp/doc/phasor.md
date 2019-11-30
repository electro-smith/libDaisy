
# phasor
Generates a normalized signal moving from 0-1 at the specified frequency.

TODO:
I'd like to make the following things easily configurable:
- Selecting which channels should be initialized/included in the sequence conversion.
- Setup a similar start function for an external mux, but that seems outside the scope of this file.

### init
Initializes the phasor module
sr, and freq are in Hz
initial phase is in radians
```c
		inline void init(float sr, float freq, float initial_phase) 
```

### process
processes phasor and returns current value

```c
		float process();
```

## Setters

### set_freq
Sets frequency of the phasor in Hz
```c
		inline void set_freq(float freq) 
```

## Getters

### get_freq
Returns current frequency value in Hz
```c
		inline float get_freq() { return _freq; }
```
