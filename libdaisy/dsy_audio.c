#ifdef __cplusplus
extern "C" {
#endif

#include "libdaisy.h"
#include "dsy_audio.h"
#include "codec_pcm3060.h"
#include "codec_wm8731.h"
#include "stm32h7xx_hal.h"
#include "dsy_dma.h"
#include "dsy_sai.h"
#include "dsy_i2c.h"
#include <string.h>

//#define DSY_PROFILE_AUDIO_CALLBACK 1

#ifdef DSY_PROFILE_AUDIO_CALLBACK
// Initialize Gate Output GPIO (only for timing in this case)
#define PROFILE_GPIO_PIN GPIO_PIN_10
#define PROFILE_GPIO_PORT GPIOG
static void init_gpio()
{
	GPIO_InitTypeDef ginit;
	ginit.Pin = PROFILE_GPIO_PIN;
	ginit.Mode = GPIO_MODE_OUTPUT_PP;
	ginit.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(PROFILE_GPIO_PORT, &ginit);
}
#endif

// Define/Declare global audio structure.
typedef struct
{
	audio_callback callback;
	int32_t		   dma_buffer_rx[DSY_AUDIO_DMA_BUFFER_SIZE];
	int32_t		   dma_buffer_tx[DSY_AUDIO_DMA_BUFFER_SIZE];
	float		   in[DSY_AUDIO_BLOCK_SIZE];
	float		   out[DSY_AUDIO_BLOCK_SIZE];
	size_t		   block_size, offset;
	uint8_t		   bitdepth, device;
	I2C_HandleTypeDef* device_control_hi2c;
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

// TODO: fix I2C to be compliant with the new model.
void dsy_audio_init(dsy_sai_handle_t* init_handle, uint8_t dev0, uint8_t dev1)
{
	uint8_t board
		= DSY_SYS_BOARD_DAISY_SEED; // just putting this here while breaking stuff.
	uint8_t intext = init_handle->init;
	//intext = 0;
	audio_handle.block_size = DSY_AUDIO_BLOCK_SIZE; // default value; todo: add configuration of this
	audio_handle_ext.block_size = DSY_AUDIO_BLOCK_SIZE; // default value
	audio_handle.callback   = dsy_audio_passthru;
	audio_handle_ext.callback   = dsy_audio_passthru;
	//if(intext == DSY_AUDIO_INTERNAL)
	if(intext == DSY_AUDIO_INIT_SAI1 || intext == DSY_AUDIO_INIT_BOTH)
	{
		audio_handle.device = dev0;
		uint8_t mcu_is_master = init_handle->sync_config[DSY_SAI_1] == DSY_AUDIO_SYNC_MASTER ? 1 : 0;
		if(dev0 == DSY_AUDIO_DEVICE_WM8731) 
		{
			//dsy_i2c2_init(board);
			codec_wm8731_init(&hi2c2, mcu_is_master, 48000.0f, 16);
		}
		else if(dev0 == DSY_AUDIO_DEVICE_PCM3060)
		{
			
		}
		for(size_t i = 0; i < DSY_AUDIO_DMA_BUFFER_SIZE; i++)
		{
			audio_handle.dma_buffer_rx[i] = 0;
			audio_handle.dma_buffer_tx[i] = 0;
		}
		for(size_t i = 0; i < DSY_AUDIO_BLOCK_SIZE; i++)
		{
			audio_handle.in[i]  = 0.0f;
			audio_handle.out[i] = 0.0f;
		}
		audio_handle.offset = 0;
	}
	if(intext == DSY_AUDIO_INIT_SAI2 || intext == DSY_AUDIO_INIT_BOTH) 
	{
		audio_handle_ext.device = dev1;
		uint8_t mcu_is_master = init_handle->sync_config[DSY_SAI_2] == DSY_AUDIO_SYNC_MASTER ? 1 : 0;
		if(dev1 == DSY_AUDIO_DEVICE_WM8731)
		{
			//dsy_i2c1_init(board);
			codec_wm8731_init(&hi2c1, mcu_is_master, 48000.0f, 16);
		}
		else if(dev1 == DSY_AUDIO_DEVICE_PCM3060)
		{
		}
		for(size_t i = 0; i < DSY_AUDIO_DMA_BUFFER_SIZE; i++)
		{
			audio_handle_ext.dma_buffer_rx[i] = 0;
			audio_handle_ext.dma_buffer_tx[i] = 0;
		}
		for(size_t i = 0; i < DSY_AUDIO_BLOCK_SIZE; i++)
		{
			audio_handle_ext.in[i]  = 0.0f;
			audio_handle_ext.out[i] = 0.0f;
		}
		audio_handle.offset = 0;
	}

	dsy_sai_init_from_handle(init_handle);

	#ifdef DSY_PROFILE_AUDIO_CALLBACK
	init_gpio();
	#endif
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
	if(intext == DSY_AUDIO_INTERNAL)
	{
		HAL_SAI_Receive_DMA(&hsai_BlockA1,
							(uint8_t*)audio_handle.dma_buffer_rx,
							DSY_AUDIO_DMA_BUFFER_SIZE);
		HAL_SAI_Transmit_DMA(&hsai_BlockB1,
							 (uint8_t*)audio_handle.dma_buffer_tx,
							 DSY_AUDIO_DMA_BUFFER_SIZE);
	}
	else
	{
		HAL_SAI_Receive_DMA(&hsai_BlockA2,
							(uint8_t*)audio_handle_ext.dma_buffer_rx,
							DSY_AUDIO_DMA_BUFFER_SIZE);
		HAL_SAI_Transmit_DMA(&hsai_BlockB2,
							 (uint8_t*)audio_handle_ext.dma_buffer_tx,
							 DSY_AUDIO_DMA_BUFFER_SIZE);
	}
}

void dsy_audio_start_streaming_output(uint8_t* buff, size_t size)
{
	//    HAL_SAI_DMAStop(&hsai_BlockA1);
	//	HAL_SAI_Transmit_DMA(&hsai_BlockA1, buff, DMA_BUFFER_SIZE);
	uint32_t* vals;
	vals			= (uint32_t*)buff;
	uint32_t offset = (audio_handle.offset + 1) * DSY_AUDIO_BLOCK_SIZE;
	for(size_t i = 0; i < size; i++)
	{
		if(i + offset < DSY_AUDIO_DMA_BUFFER_SIZE)
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

// If the device supports hardware bypass, enter that mode.
void dsy_audio_enter_bypass(uint8_t intext) 
{
	if(intext == DSY_AUDIO_INTERNAL) 
	{
		switch(audio_handle.device)
		{
			case DSY_AUDIO_DEVICE_WM8731: 
				codec_wm8731_enter_bypass(audio_handle.device_control_hi2c); 
				break;
			default: break;
		}
	}
}

// If the device supports hardware bypass, exit that mode.
void dsy_audio_exit_bypass(uint8_t intext) 
{
	if(intext == DSY_AUDIO_INTERNAL) 
	{
		switch(audio_handle.device)
		{
			case DSY_AUDIO_DEVICE_WM8731: 
				codec_wm8731_exit_bypass(audio_handle.device_control_hi2c); 
				break;
			default: break;
		}
	}
}
// Static Functions Below

static void internal_callback(SAI_HandleTypeDef* hsai, size_t offset)
{
	#ifdef DSY_PROFILE_AUDIO_CALLBACK
	HAL_GPIO_WritePin(PROFILE_GPIO_PORT, PROFILE_GPIO_PIN, 0);
	#endif
	et_audio_t* ah = get_audio_from_sai(hsai);

	const int32_t* ini  = ah->dma_buffer_rx + offset;
	float*		   inf  = ah->in;
	const float*   endi = ah->in + ah->block_size;
//	SCB_InvalidateDCache_by_Addr((uint32_t *)((uint32_t)(ini) & ~(uint32_t) 0x1F), ah->block_size * 4);
	if(ah->bitdepth == DSY_AUDIO_BITDEPTH_24)
	{
		while(inf != endi)
		{
			*inf++ = s242f(*ini++);
		}
	}
	else
	{
		while(inf != endi) 
		{
			*inf++ = s162f(*ini++);
		}
	}

	ah->callback(ah->in, ah->out, ah->block_size);
	int32_t*	 outi = ah->dma_buffer_tx + offset;
	const float* outf = ah->out;
	const float* endo = ah->out + ah->block_size;
	if(ah->bitdepth == DSY_AUDIO_BITDEPTH_24)
	{
		while(outf != endo)
		{
			*outi++ = f2s24(*outf++);
		}
	}
	else
	{
		while(outf != endo)
		{
			*outi++ = f2s16(*outf++);
		}
	}
	#ifdef DSY_PROFILE_AUDIO_CALLBACK
	HAL_GPIO_WritePin(PROFILE_GPIO_PORT, PROFILE_GPIO_PIN, 1);
	#endif
}

// Tests


// DMA Callbacks
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef* hsai)
{
	internal_callback(hsai, 0);
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef* hsai)
{
	internal_callback(hsai, audio_handle.block_size);
}

#ifdef __cplusplus
}
#endif
