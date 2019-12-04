
# crossfade
Performs a crossfade between two signals

Original author: Paul Batchelor

Ported from Soundpipe by Andrew Ikenberry
added curve option for constant power, etc.

TODO:
- implement constant power curve process
- implement exponential curve process
- implement logarithmic curve process

## Curve Options
Curve applied to the crossfade
- LIN = linear
- CPOW = constant power
- LOG = logarithmic
- EXP  exponential
- LAST = end of enum (used for array indexing)
```c
	enum 
	{
		CROSSFADE_LIN,
		CROSSFADE_CPOW,
		CROSSFADE_LOG,
		CROSSFADE_EXP,
		CROSSFADE_LAST,
	};
```

### init
Initializes crossfade module

Defaults
- current position = .5
- curve = linear

```c
		inline void init() 
```

### process
processes crossfade and returns single sample 

```c
		float process(float *in1, float *in2);
```

## Setters

### set_pos
Sets position of crossfade between two input signals
Input range: 0 to 1
```c
		inline void set_pos(float pos) { _pos = pos; }
```

### set_curve
Sets current curve applied to crossfade 
Expected input: See [Curve Options](##curve-options)
```c
		inline void set_curve(uint8_t curve) { _curve = curve; }
```

## Getters

### get_pos
Returns current position
```c
		inline float get_pos(float pos) { return _pos; }
```

### get_curve
Returns current curve
```c
		inline uint8_t get_curve(uint8_t curve) { return _curve; }
```
