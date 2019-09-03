// WM8371 Codec support.

#ifndef SA_DRIVERS_CODEC_H_
#define SA_DRIVERS_CODEC_H_

//#include <stm32f4xx_conf.h>
#include <inttypes.h>
#include <stddef.h>
//#include "main.h"
#include "sai.h"
#include "i2c.h"
#include "codec_wm8731_frame.h"
#define SA_CODEC_BLOCK_SIZE_MAX 24


typedef void (*sa_codec_fill_buffer_callback)( \
	codec_frame_t* rx, \
	codec_frame_t* tx, \
	size_t size);

//#ifndef SA_AUDIO_CALLBACK
//#define SA_AUDIO_CALLBACK
//typedef struct {
//	short l;
//	short r;
//}codec_frame_t;
//typedef void (*sa_audio_callback)(codec_frame_t* , codec_frame_t* , size_t);
//#endif

typedef struct {
  uint8_t mcu_is_master;
  int32_t sample_rate;
  size_t block_size;
  size_t stride;
  //sa_codec_fill_buffer_callback callback;
  sa_audio_callback process;
  //DMA_InitTypeDef dma_init_tx;
  //DMA_InitTypeDef dma_init_rx;
  short tx_dma_buffer[SA_CODEC_BLOCK_SIZE_MAX * 8 * 2];
  short rx_dma_buffer[SA_CODEC_BLOCK_SIZE_MAX * 8 * 2];
}sa_codec;

uint8_t sa_codec_init( \
	uint8_t mcu_is_master, \
	int32_t sample_rate);

uint8_t sa_codec_start(sa_codec* c, size_t block_size);
uint8_t sa_codec_start_callback(
        sa_codec* c,
        size_t block_size, 
        sa_codec_fill_buffer_callback callback);

int sa_audio_start_callback(
        sa_codec* c,
        int block_size, 
        sa_audio_callback process);
void sa_codec_stop(sa_codec* c);
void sa_codec_fill(sa_codec* c, size_t offset);
void sa_audio_fill(sa_codec* c, size_t offset);
uint8_t sa_codec_set_line_input_gain(sa_codec* c, int32_t channel, int32_t gain);
uint8_t sa_codec_set_stereo_line_input_gain(sa_codec* c, int32_t gain);

// these were private functions in the codec class.
uint8_t sa_codec_initialize_gpio();
uint8_t sa_codec_initialize_control_interface();
uint8_t sa_codec_initialize_audio_interface(uint8_t mcu_is_master, int32_t sample_rate);
uint8_t sa_codec_initialize_codec(uint8_t mcu_is_master, int32_t sample_rate);
uint8_t sa_codec_initialize_dma(sa_codec* c);
uint8_t sa_codec_write_control_register(uint8_t address, uint16_t data);

#endif
