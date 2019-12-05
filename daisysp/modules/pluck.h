//
// # pluck
//
// Produces a naturally decaying plucked string or drum sound based on the Karplus-Strong algorithms.
// 
// This code has been extracted from the Csound opcode "pluck"
// It has been modified to work as a Daisy Soundpipe module.
// 
// Original Author(s): Barry Vercoe, John ffitch
//
// Year: 1991
//
// Location: OOps/ugens4.c
//

#pragma once
#ifndef DSY_PLUCK_H
#define DSY_PLUCK_H

#include <stdint.h>
#ifdef __cplusplus

namespace daisysp
{

// ## Mode
// The method of natural decay that the algorithm will use.
//
// - RECURSIVE: 1st order recursive filter, with coefs .5.
// - WEIGHTED_AVERAGE: weighted averaging. 
// ~~~~
	enum 
	{
		PLUCK_MODE_RECURSIVE,
		PLUCK_MODE_WEIGHTED_AVERAGE,
	};
// ~~~~

	class pluck
	{
	public:
		pluck() {}
		~pluck() {}

// ### init
// 
// Initializes the Pluck module.
//
// Arguments:
// 
// - sample_rate: Sample rate of the audio engine being run.
// - buf: Buffer for ?
// - npt: 
// - mode: Sets the mode of the algorithm.
// 
// ~~~~
		void init (float sample_rate, float *buf, int32_t npt, int32_t mode);
// ~~~~

// ### process
//
// Processes the waveform to be generated, returning one sample. This should be called once per sample period.
// ~~~~
		float process (float &trig);
// ~~~~

// ## Mutators
// 
// ### set_amp
// 
// Sets the amplitude of the output signal.
// 
// Input range: 0-1?
// 
// ~~~~
		inline void set_amp(float amp) { amp_ = amp; }
// ~~~~

// ### set_freq
// 
// Sets the frequency of the output signal in Hz.
// 
// Input range: Any positive value
// 
// ~~~~
		inline void set_freq(float freq) { freq_ = freq; }
// ~~~~

// ### set_decay
// 
// Sets the time it takes for a triggered note to end in seconds.
// 
// Input range: Any positive value
// 
// ~~~~
		inline void set_decay(float decay) { decay_ = decay; }
// ~~~~

// ### set_mode
// 
// Sets the mode of the algorithm.
// 
// ~~~~
		inline void set_mode(int32_t mode) { mode_ = mode; }
// ~~~~

// ## Accessors
// 
// ### get_amp
// 
// Returns the current value for amp.
// 
// ~~~~
		inline float get_amp() { return amp_; }
// ~~~~

// ### get_freq
// 
// Returns the current value for freq.
// 
// ~~~~
		inline float get_freq() { return freq_; }
// ~~~~

// ### get_decay
// 
// Returns the current value for decay.
// 
// ~~~~
		inline float get_decay() { return decay_; }
// ~~~~

// ### get_mode
// 
// Returns the current value for mode.
// 
// ~~~~
		inline int32_t get_mode() { return mode_; }
// ~~~~

	private:
		void reinit();
		float amp_, freq_, decay_, damp_, ifreq_;
	    float sicps_;
	    int32_t phs256_, npts_, maxpts_;
	    float *buf_;
	    float sample_rate_;
	    char init_;
	    int32_t mode_;
	};
} // namespace daisysp
#endif
#endif
