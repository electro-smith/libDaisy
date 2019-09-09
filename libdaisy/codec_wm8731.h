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

// TODO: Move dsy_wm8731_handle_t to be a static var in the c file, and trim fat (buffers, etc)



uint8_t codec_wm8731_init( \
	I2C_HandleTypeDef *hi2c, \
	uint8_t mcu_is_master, \
	int32_t sample_rate);

//uint8_t sa_codec_start(dsy_wm8731_handle_t* c, size_t block_size);
//uint8_t sa_codec_start_callback(
//        dsy_wm8731_handle_t* c,
//        size_t block_size, 
//        sa_codec_fill_buffer_callback callback);
//
//int sa_audio_start_callback(
//        dsy_wm8731_handle_t* c,
//        int block_size, 
//        sa_audio_callback process);
//void sa_codec_stop(dsy_wm8731_handle_t* c);
//void sa_codec_fill(dsy_wm8731_handle_t* c, size_t offset);
//void sa_audio_fill(dsy_wm8731_handle_t* c, size_t offset);
//uint8_t sa_codec_set_line_input_gain(dsy_wm8731_handle_t* c, int32_t channel, int32_t gain);
//uint8_t sa_codec_set_stereo_line_input_gain(dsy_wm8731_handle_t* c, int32_t gain);
//
//// these were private functions in the codec class.
//uint8_t sa_codec_initialize_gpio();
//uint8_t sa_codec_initialize_control_interface();
//uint8_t sa_codec_initialize_audio_interface(uint8_t mcu_is_master, int32_t sample_rate);
//uint8_t sa_codec_initialize_codec(uint8_t mcu_is_master, int32_t sample_rate);
//uint8_t sa_codec_initialize_dma(dsy_wm8731_handle_t* c);

#endif
