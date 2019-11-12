#ifndef DSY_AUDIO_H
#define DSY_AUDIO_H
#include <stddef.h>
#include <stdint.h>
#include "dsy_sai.h"
#include "dsy_i2c.h"
#ifdef __cplusplus
extern "C"
{
#endif

// TODO Fix hard coding of these parameters
#ifndef DSY_AUDIO_SAMPLE_RATE
//#define SAMPLE_RATE 47916.0f
//#define SAMPLE_RATE 31941.0f
//#define SAMPLE_RATE 48014.0f
#define DSY_AUDIO_SAMPLE_RATE 48014.0f
//#define SAMPLE_RATE 95820.0f
#endif

#ifdef AUDIO_TOTAL_BUF_SIZE
#define DMA_BUFFER_SIZE AUDIO_TOTAL_BUF_SIZE / 2
#else

#ifndef DSY_AUDIO_DMA_BUFFER_SIZE
//#define DSY_AUDIO_DMA_BUFFER_SIZE 48
#define DSY_AUDIO_DMA_BUFFER_SIZE 64
//#define DMA_BUFFER_SIZE 128
#endif // DMA_BUFFER_SIZE

#endif // AUDIO_TOTAL_BUF_SIZE

#define DSY_AUDIO_BLOCK_SIZE (DSY_AUDIO_DMA_BUFFER_SIZE / 2)

	// Thinking about getting rid of this...

	// If initialized to a single channel, its just that.
	// If both initialized, then you get a quad callback.
	enum
	{
		DSY_AUDIO_INTERNAL,
		DSY_AUDIO_EXTERNAL,
		DSY_AUDIO_LAST,
	};

	typedef void (*audio_callback)(float*, float*, size_t);
	void dsy_audio_init(dsy_sai_handle_t* sai_handle,
						dsy_i2c_handle_t* dev0_i2c,
						dsy_i2c_handle_t* dev1_i2c);
	void dsy_audio_set_callback(uint8_t intext, audio_callback cb);

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