#ifndef DSY_AUDIO_H
#define DSY_AUDIO_H
#include <stddef.h>
#include <stdint.h>

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
#define DSY_AUDIO_DMA_BUFFER_SIZE 96
//#define DMA_BUFFER_SIZE 128
#endif // DMA_BUFFER_SIZE

#endif // AUDIO_TOTAL_BUF_SIZE

#define DSY_AUDIO_BLOCK_SIZE (DSY_AUDIO_DMA_BUFFER_SIZE / 2)

enum
{
	DSY_AUDIO_INTERNAL,
	DSY_AUDIO_EXTERNAL,
	DSY_AUDIO_LAST,
};

enum
{
	DSY_AUDIO_DEVICE_PCM3060,
	DSY_AUDIO_DEVICE_WM8731,
	DSY_AUDIO_DEVICE_LAST,
};

// Stereo input/output buffer
// float *input *output
// size_t number of samples
typedef void (*audio_callback)(float*, float*, size_t);

void dsy_audio_init(uint8_t board, uint8_t intext, uint8_t device);
void dsy_audio_set_callback(uint8_t intext, audio_callback cb);
void dsy_audio_start(uint8_t intext);
void dsy_audio_stop(uint8_t intext);

// If the device supports hardware bypass, enter that mode.
void dsy_audio_enter_bypass(uint8_t intext);

// If the device supports hardware bypass, exit that mode.
void dsy_audio_exit_bypass(uint8_t intext);

void dsy_audio_passthru(float* in, float* out, size_t size);
void dsy_audio_silence(float* in, float* out, size_t size);

#endif