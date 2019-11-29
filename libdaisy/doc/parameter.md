
# Parameter

     Simple parameter mapping tool that takes a 0-1 input from an hid_ctrl.

TODO: Move init and process to .cpp file
- i was cool with them being in the h file until math.h got involved for the log stuff.

### curve settings
Curves are applied to the output signal
```c
	enum
	{
		PARAM_CURVE_LINEAR,
		PARAM_CURVE_EXP,
		PARAM_CURVE_LOG,
		PARAM_CURVE_CUBE,
		PARAM_CURVE_LAST,
	};
```

## parameter class

### init
initialize a parameter using an hid_ctrl object.

hid_ctrl input - object containing the direct link to a hardware control source.

min - bottom of range. (when input is 0.0)

max - top of range (when input is 1.0)

curve - the scaling curve for the input->output transformation.
~~~
~~~

### process
processes the input signal, this should be called at the samplerate of the hid_ctrl passed in.

returns a float with the specified transformation applied.
```c
		inline float process()
```

### value
returns the current value from the parameter without processing another sample.
this is useful if you need to use the value multiple times, and don't store 
the output of process in a local variable.
```c
		inline float value() { return val; }
```
