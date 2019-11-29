
# oscillator

     Synthesis of several waveforms, including polyBLEP bandlimited waveforms.

example:

```c
daisysp::oscillator osc;
init()
{
    osc.init(SAMPLE_RATE);
    osc.set_frequency(440);
    osc.set_amp(0.25);
    osc.set_waveform(WAVE_TRI);
}

callback(float *in, float *out, size_t size)
{
    for (size_t i = 0; i < size; i+=2)
    {
        out[i] = out[i+1] = osc.process();
    }
}
```

## Waveforms

Choices for output waveforms, POLYBLEP are appropriately labeled. Others are naive forms.
```c
		enum
		{
			WAVE_SIN,	
			WAVE_TRI,
			WAVE_SAW,
			WAVE_RAMP,
			WAVE_SQUARE,
			WAVE_POLYBLEP_TRI,
			WAVE_POLYBLEP_SAW,
			WAVE_POLYBLEP_SQUARE,
			WAVE_LAST,
		};
```

### init

Initializes the oscillator 

float samplerate - sample rate of the audio engine being run, and the frequency that the process function will be called.

Defaults:
- freq = 100 Hz
- amp = 0.5 
- waveform = sine wave.

```c
		void init(float samplerate)
```

### set_freq

Changes the frequency of the oscillator, and recalculates phase increment.
```c
		inline void set_freq(const float f) 
```

### set_amp

Sets the amplitude of the waveform.
```c
		inline void set_amp(const float a) { amp = a; }
```

### set_waveform

Sets the waveform to be synthesized by the process() function.
```c
		inline void set_waveform(const uint8_t wf) { waveform = wf < WAVE_LAST ? wf : WAVE_SIN; }
```

### process

Processes the waveform to be generated, returning one sample. This should be called once per sample period.
```c
		float process();
```
