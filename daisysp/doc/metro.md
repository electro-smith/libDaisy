
# metro 
Creates a clock signal at specific frequency

### init
Initializes metro module
Arguments:
- freq: frequency at which new clock signals will be generated
	Input Range: 
- sample_rate: sample rate of audio engine
	Input range: 
```c
		void init(float freq, float sample_rate);
```

### process
checks current state of metro object and updates state if necesary.
```c
		uint8_t process();
```

## Setters

### set_freq
Sets frequency at which metro module will run at.
```c
		inline void set_freq(float freq) 
```

## Getters

### get_freq
Returns current value for frequency.
```c
		inline float get_freq() { return _freq; }
```
