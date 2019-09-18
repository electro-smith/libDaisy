#include "libdaisy.h"
#include "dsy_audio.h"
#include "codec_pcm3060.h"
#include "codec_wm8731.h"
#include "stm32h7xx_hal.h"
#include "dma.h"
#include "sai.h"
#include "i2c.h"

#include <string.h>

// Define/Declare global audio structure.
typedef struct
{
	audio_callback callback;
	int16_t		   dma_buffer_rx[DMA_BUFFER_SIZE];
	int16_t		   dma_buffer_tx[DMA_BUFFER_SIZE];
	float		   in[BLOCK_SIZE];
	float		   out[BLOCK_SIZE];
	size_t		   block_size;
	size_t		   offset;
} et_audio_t;
static et_audio_t audio_handle;
static et_audio_t audio_handle_ext;

static FORCE_INLINE et_audio_t* get_audio_from_sai(SAI_HandleTypeDef* hsai)
{
	return (hsai->Instance == SAI1_Block_A) ? &audio_handle : &audio_handle_ext;
}

extern DMA_HandleTypeDef hdma_sai1_a;
extern DMA_HandleTypeDef hdma_sai1_b;
extern DMA_HandleTypeDef hdma_sai2_a;
extern DMA_HandleTypeDef hdma_sai2_b;

void DMA1_Stream0_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_sai1_a);
}

void DMA1_Stream1_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_sai1_b);
}

//void DMA1_Stream1_IRQHandler(void)
//{
//	HAL_DMA_IRQHandler(&hdma_sai1_a);
//}
//
//void DMA1_Stream2_IRQHandler(void)
//{
//	HAL_DMA_IRQHandler(&hdma_sai1_b);
//}

void DMA1_Stream3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_sai2_a);
}
void DMA1_Stream4_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_sai2_b);
}


void dsy_audio_passthru(float* in, float* out, size_t size)
{
	memcpy(out, in, sizeof(float) * size);
}

void dsy_audio_silence(float* in, float* out, size_t size)
{
	memset(out, 0, sizeof(float) * size);
}

// TODO Init only the channel being used to save memory
void dsy_audio_init(uint8_t board, uint8_t intext, uint8_t device)
{
	MX_DMA_Init();
	MX_SAI1_Init();
	MX_SAI2_Init();

	if(intext == DSY_AUDIO_INTERNAL)
	{
		if(board == DSY_SYS_BOARD_DAISY_SEED)
		{
			MX_I2C2_Init();
			codec_wm8731_init(&hi2c2, 1, 48000.0f);
		}
		else if(board == DSY_SYS_BOARD_AUDIO_BB)
		{
			MX_I2C1_Init();
			codec_wm8731_init(&hi2c1, 1, 48000.0f);
		}
		else
		{
			MX_I2C2_Init();
			codec_pcm3060_init(&hi2c2);
		}
	}
	else
	{
		switch(device)
		{
			case DSY_AUDIO_DEVICE_PCM3060:
				MX_I2C1_Init();
				codec_pcm3060_init(&hi2c1);
				break;
			case DSY_AUDIO_DEVICE_WM8731:
				MX_I2C1_Init();
				codec_wm8731_init(&hi2c1, 0, 48000.0f);
				break;
			default: break;
		}
	}
	// Initialize Internal Audio Handle
	audio_handle.callback   = dsy_audio_passthru;
	audio_handle.block_size = BLOCK_SIZE;

	for(size_t i = 0; i < DMA_BUFFER_SIZE; i++)
	{
		audio_handle.dma_buffer_rx[i] = 0;
		audio_handle.dma_buffer_tx[i] = 0;
	}
	for(size_t i = 0; i < BLOCK_SIZE; i++)
	{
		audio_handle.in[i]  = 0.0f;
		audio_handle.out[i] = 0.0f;
	}
	audio_handle.offset = 0;
	// Initialize External Audio Handle
	audio_handle_ext.callback   = dsy_audio_passthru;
	audio_handle_ext.block_size = BLOCK_SIZE;
	for(size_t i = 0; i < DMA_BUFFER_SIZE; i++)
	{
		audio_handle_ext.dma_buffer_rx[i] = 0;
		audio_handle_ext.dma_buffer_tx[i] = 0;
	}
	for(size_t i = 0; i < BLOCK_SIZE; i++)
	{
		audio_handle_ext.in[i]  = 0.0f;
		audio_handle_ext.out[i] = 0.0f;
	}
	audio_handle_ext.offset = 0;
#ifdef __USBD_AUDIO_IF_H__
	audio_start(); // start audio callbacks, and then we'll see if it works or not...
#endif
}

void dsy_audio_set_callback(uint8_t intext, audio_callback cb)
{
	if(intext == DSY_AUDIO_INTERNAL)
	{
		audio_handle.callback = cb;
	}
	else
	{
		audio_handle_ext.callback = cb;
	}
}

void dsy_audio_start(uint8_t intext)
{
	//    HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)audio_handle.dma_buffer_rx, DMA_BUFFER_SIZE);
	//    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audio_handle.dma_buffer_tx, DMA_BUFFER_SIZE);
	if(intext == DSY_AUDIO_INTERNAL)
	{
		HAL_SAI_Receive_DMA(&hsai_BlockA1,
							(uint8_t*)audio_handle.dma_buffer_rx,
							DMA_BUFFER_SIZE);
		HAL_SAI_Transmit_DMA(&hsai_BlockB1,
							 (uint8_t*)audio_handle.dma_buffer_tx,
							 DMA_BUFFER_SIZE);
	}
	else
	{
		HAL_SAI_Receive_DMA(&hsai_BlockA2,
							(uint8_t*)audio_handle_ext.dma_buffer_rx,
							DMA_BUFFER_SIZE);
		HAL_SAI_Transmit_DMA(&hsai_BlockB2,
							 (uint8_t*)audio_handle_ext.dma_buffer_tx,
							 DMA_BUFFER_SIZE);
	}
}

void dsy_audio_start_streaming_output(uint8_t* buff, size_t size)
{
	//    HAL_SAI_DMAStop(&hsai_BlockA1);
	//	HAL_SAI_Transmit_DMA(&hsai_BlockA1, buff, DMA_BUFFER_SIZE);
	uint32_t* vals;
	vals			= (uint32_t*)buff;
	uint32_t offset = (audio_handle.offset + 1) * BLOCK_SIZE;
	for(size_t i = 0; i < size; i++)
	{
		if(i + offset < DMA_BUFFER_SIZE)
		{
			audio_handle.dma_buffer_tx[i + offset] = vals[i] << 8;
		}
	}

	audio_handle.offset += 1;
	if(audio_handle.offset > 1)
	{
		audio_handle.offset = 0;
	}
	SCB_InvalidateDCache();
}

void audio_stop(uint8_t intext)
{
	if(intext == DSY_AUDIO_INTERNAL)
	{
		HAL_SAI_DMAStop(&hsai_BlockA1);
		HAL_SAI_DMAStop(&hsai_BlockB1);
	}
	else
	{
		HAL_SAI_DMAStop(&hsai_BlockA2);
		HAL_SAI_DMAStop(&hsai_BlockB2);
	}
}

static void internal_callback(SAI_HandleTypeDef* hsai, size_t offset)
{
	et_audio_t* ah = get_audio_from_sai(hsai);

	const int16_t* ini  = ah->dma_buffer_rx + offset;
	float*		   inf  = ah->in;
	const float*   endi = ah->in + ah->block_size;

	while(inf != endi)
	{
		*inf++ = s162f(*ini++);
	}

	ah->callback(ah->in, ah->out, ah->block_size);

	int16_t*	 outi = ah->dma_buffer_tx + offset;
	const float* outf = ah->out;
	const float* endo = ah->out + ah->block_size;

	while(outf != endo)
	{
		*outi++ = f2s16(*outf++);
	}
}

// DMA Callbacks
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef* hsai)
{
	internal_callback(hsai, 0);
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef* hsai)
{
	internal_callback(hsai, audio_handle.block_size);
}
