
# decimator
Performs downsampling and bitcrush effects

### init
Initializes downsample module
```c
		void init();
```

### process
Applies downsample and bitcrush effects to input signal
```c
		float process(float input);
```

## Setters

### set_downsample_factor
Sets amount of downsample 
Input range: 
```c
		inline void set_downsample_factor (float downsample_factor) 
```

### set_bitcrush_factor
Sets amount of bitcrushing 
Input range: 
```c
		inline void set_bitcrush_factor (float bitcrush_factor)
```

## Getters

### get_downsample_factor
Returns current setting of downsample
```c
		inline float get_downsample_factor () { return _downsample_factor; }
```

### get_bitcrush_factor
Returns current setting of bitcrush
```c
		inline float get_bitcrush_factor () { return _bitcrush_factor; }
```
