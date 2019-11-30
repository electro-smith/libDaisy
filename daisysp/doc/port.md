
# port
Applies portamento to an input signal

This code has been ported from Soundpipe to DaisySP by (paul batchelor?). 
The Soundpipe module was extracted from the Csound opcode "portk".
Original Author(s): Robbin Whittle, John ffitch
Year: 1995, 1998
Location: Opcodes/biquad.c

### init
Initializes port module
Arguments:
- sr: sample rate of audio engine
- htime: time of portamento
```c
		void init(int sr, float htime);
```

### process
Applies portamento to input signal and returns processed signal. 
```c
		float process(float in);
```

## Setters

### set_htime
Sets htime
```c
		inline void set_htime(float htime) { _htime = htime; }
```

## Getters

### get_htime
returns current value of htime
```c
		inline float get_htime() { return _htime; }
```
