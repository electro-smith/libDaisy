
# line
creates a line segment signal

### init
Initializes line module.
```c
		void init(float sample_rate);
```

### process
Processes line segment. Returns one sample.
Expected input: 
```c
		float process(uint8_t *finished);
```

### start
Begin creation of line. 
Arguments:
- start - beginning value
- end - ending value
- dur - duration in seconds of line segment
```c
		void start(float start, float end, float dur);
```
