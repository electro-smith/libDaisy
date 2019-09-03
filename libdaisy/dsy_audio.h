#ifndef DSY_AUDIO_H
#define DSY_AUDIO_H
#include <stddef.h>
#include <stdint.h>

// TODO Fix hard coding of these parameters
#ifndef SAMPLE_RATE
//#define SAMPLE_RATE 47916.0f
//#define SAMPLE_RATE 31941.0f
//#define SAMPLE_RATE 48014.0f
#define SAMPLE_RATE 48014.0f
//#define SAMPLE_RATE 95820.0f
#endif

#ifdef AUDIO_TOTAL_BUF_SIZE
#define DMA_BUFFER_SIZE AUDIO_TOTAL_BUF_SIZE / 2
#else

#ifndef DMA_BUFFER_SIZE
#define DMA_BUFFER_SIZE 48
#endif // DMA_BUFFER_SIZE

#endif // AUDIO_TOTAL_BUF_SIZE

#define BLOCK_SIZE (DMA_BUFFER_SIZE/2)

// Stereo input/output buffer
// float *input *output 
// size_t number of samples
typedef void (*audio_callback)(float*, float*, size_t);

void dsy_audio_init(uint8_t board);
void dsy_audio_set_callback(audio_callback cb);
void dsy_audio_start();
void dsy_audio_stop();

#endif