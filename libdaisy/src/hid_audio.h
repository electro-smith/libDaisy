#ifndef DSY_AUDIO_H
#define DSY_AUDIO_H
#include <stddef.h>
#include <stdint.h>
#include "per_sai.h"
#include "per_i2c.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define DSY_AUDIO_BLOCK_SIZE_MAX 128
#define DSY_AUDIO_CHANNELS_MAX 2

// TODO Fix hard coding of these parameters
#ifndef DSY_AUDIO_SAMPLE_RATE
#define DSY_AUDIO_SAMPLE_RATE 48014.0f
#endif

	// Thinking about getting rid of this...

	// If initialized to a single channel, its just that.
	// If both initialized, then you get a quad callback.
	enum
	{
		DSY_AUDIO_INTERNAL,
		DSY_AUDIO_EXTERNAL,
		DSY_AUDIO_LAST,
	};

	typedef struct
	{
		size_t			block_size;
		dsy_sai_handle *sai;
		dsy_i2c_handle *dev0_i2c;
		dsy_i2c_handle *dev1_i2c;
	} dsy_audio_handle;

	typedef void (*dsy_audio_callback)(float*, float*, size_t);
	void dsy_audio_init(dsy_audio_handle* handle);
	void dsy_audio_set_callback(uint8_t intext, dsy_audio_callback cb);
	void dsy_audio_set_blocksize(uint8_t intext, size_t blocksize);

	void dsy_audio_start(uint8_t intext);

	// Stops transmitting/receiving audio.
	void dsy_audio_stop(uint8_t intext);

	// If the device supports hardware bypass, enter that mode.
	void dsy_audio_enter_bypass(uint8_t intext);

	// If the device supports hardware bypass, exit that mode.
	void dsy_audio_exit_bypass(uint8_t intext);

	// Default Callbacks
	void dsy_audio_passthru(float* in, float* out, size_t size);
	void dsy_audio_silence(float* in, float* out, size_t size);

#ifdef __cplusplus
}
#endif
#endif
