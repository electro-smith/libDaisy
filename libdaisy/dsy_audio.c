#include <libdaisy.h>
#include "dsy_audio.h"
#include "codec_pcm3060.h"
#include "codec_wm8731.h"
#include "stm32h7xx_hal.h"
#include "dma.h"
#include "sai.h"
#include "i2c.h"

// For bypass testing of audio circuit.
//#define TEST_BYPASS 1

// signed 24-bit maximum (2^23)
//#ifndef MAX_AMP 
//#define MAX_AMP 8388607.0f
//#endif
//#ifndef MAX_OUT_AMP 
//#define MAX_OUT_AMP 8388607.0f
//#endif
//#ifndef MAX_IN_AMP
//#define MAX_IN_AMP 2147483648.0f
//#endif
//#define MAX_AMP 65535.0f
//#define MAX_OUT_AMP 65535.0f
//#define MAX_IN_AMP 65535.0f
#define MAX_AMP 32767.0f
#define MAX_OUT_AMP 32767.0f
#define MAX_IN_AMP 32767.0f

// Define/Declare global audio structure. 
typedef struct
{
    audio_callback callback;
    int16_t dma_buffer_rx[DMA_BUFFER_SIZE];
    int16_t dma_buffer_tx[DMA_BUFFER_SIZE];
    float in[BLOCK_SIZE];
    float out[BLOCK_SIZE];
    size_t block_size;
	size_t offset;
}et_audio_t;
static et_audio_t audio_handle;
static et_audio_t audio_handle_ext;

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

static void empty(float *in, float *out, size_t size)
{
    size *= 2;
    for (size_t i = 0; i < size; i+=2)
    {
//        in[i] = 0.0f;
//        in[i+1] = 0.0f;
        out[i] = in[i];
        out[i+1] = in[i+1];
    }
}

void dsy_audio_init(uint8_t board, uint8_t intext, uint8_t device)
{
	// For now expecting external MX_SAI_Init() is being run in main.c
	MX_DMA_Init();
	MX_SAI1_Init();
	MX_SAI2_Init();
	MX_I2C2_Init();
	if (intext == DSY_AUDIO_INTERNAL)
	{
		if (board == DSY_SYS_BOARD_DAISY_SEED)
		{
			sa_codec_init(&hi2c2 , 1, 48000.0f);
		}
		else
		{
			codec_pcm3060_init(&hi2c2);
		}
		
	}
	else
	{
		switch (device)
		{
		case DSY_AUDIO_DEVICE_PCM3060:
			codec_pcm3060_init(&hi2c1);
			break;
		case DSY_AUDIO_DEVICE_WM8731:
			sa_codec_init(&hi2c1, 0, 48000.0f);
			break;
		default:
			break;
		}
	}
	// Initialize Internal Audio Handle
    audio_handle.callback = empty;
    audio_handle.block_size = BLOCK_SIZE;
    for (size_t i = 0; i < DMA_BUFFER_SIZE; i++) 
    {
        audio_handle.dma_buffer_rx[i] = 0;
        audio_handle.dma_buffer_tx[i] = 0;
    }
    for (size_t i = 0; i < BLOCK_SIZE; i++)
    {
        audio_handle.in[i] = 0.0f;
        audio_handle.out[i] = 0.0f;
    }
	audio_handle.offset = 0;
	// Initialize External Audio Handle
    audio_handle_ext.callback = empty;
    audio_handle_ext.block_size = BLOCK_SIZE;
    for (size_t i = 0; i < DMA_BUFFER_SIZE; i++) 
    {
        audio_handle_ext.dma_buffer_rx[i] = 0;
        audio_handle_ext.dma_buffer_tx[i] = 0;
    }
    for (size_t i = 0; i < BLOCK_SIZE; i++)
    {
        audio_handle_ext.in[i] = 0.0f;
        audio_handle_ext.out[i] = 0.0f;
    }
	audio_handle_ext.offset = 0;
	#ifdef __USBD_AUDIO_IF_H__
	audio_start(); // start audio callbacks, and then we'll see if it works or not...
	#endif
}

void dsy_audio_set_callback(uint8_t intext, audio_callback cb)
{
	if (intext == DSY_AUDIO_INTERNAL)
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
		HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)audio_handle.dma_buffer_rx, DMA_BUFFER_SIZE / 2);
		HAL_SAI_Transmit_DMA(&hsai_BlockB1, (uint8_t *)audio_handle.dma_buffer_tx, DMA_BUFFER_SIZE / 2);
	}
	else
	{
		HAL_SAI_Receive_DMA(&hsai_BlockA2, (uint8_t *)audio_handle_ext.dma_buffer_rx, DMA_BUFFER_SIZE);
		HAL_SAI_Transmit_DMA(&hsai_BlockB2, (uint8_t *)audio_handle_ext.dma_buffer_tx, DMA_BUFFER_SIZE);
	}
}
void dsy_audio_start_streaming_output(uint8_t *buff, size_t size)
{
	//    HAL_SAI_DMAStop(&hsai_BlockA1);
	//	HAL_SAI_Transmit_DMA(&hsai_BlockA1, buff, DMA_BUFFER_SIZE);
	uint32_t *vals;
	vals = (uint32_t*)buff;
	uint32_t offset = (audio_handle.offset + 1) * BLOCK_SIZE;
	for(size_t i = 0 ; i < size ; i++)
	{
		if (i + offset < DMA_BUFFER_SIZE)
		{
			audio_handle.dma_buffer_tx[i + offset] = vals[i] << 8;
		}
	}
	audio_handle.offset += 1;
	if (audio_handle.offset > 1)
	{
		
		audio_handle.offset = 0;
	}
	SCB_InvalidateDCache();
}

void audio_stop(uint8_t intext)
{
	if (intext == DSY_AUDIO_INTERNAL)
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


#ifdef TEST_BYPASS
static void test_bypass(int32_t offset);
#endif
static void internal_callback(SAI_HandleTypeDef* hsai, uint8_t intext, size_t offset)
{
	et_audio_t *ah;
	if (intext == DSY_AUDIO_INTERNAL)
	{
		ah = &audio_handle;	
	}
	else
	{
		ah = &audio_handle_ext;
	}
	for (size_t i = 0; i < audio_handle.block_size; i += 2)
	{
		ah->in[i] = (float)((((int16_t)ah->dma_buffer_rx[offset + i]))  / MAX_AMP);
		ah->in[i + 1] = (float)((((int16_t)ah->dma_buffer_rx[offset + i + 1])) / MAX_AMP);
	}
	if (ah->callback)
	{
		(ah->callback)((float*)ah->in,(float*)ah->out,(size_t)ah->block_size);
	}
	for (size_t i = 0; i < audio_handle.block_size; i+=2)
	{
		ah->dma_buffer_tx[offset + i] = (int16_t)(ah->out[i] * MAX_OUT_AMP);
		ah->dma_buffer_tx[offset + i + 1] = (int16_t)(ah->out[i+1] * MAX_OUT_AMP);
	}
}
// DMA Callbacks
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef* hsai)
{
	size_t offset = 0;
#ifdef TEST_BYPASS
		//HalfTransfer_CallBack_FS();
	test_bypass(offset);
#else
	if (hsai->Instance == SAI1_Block_A)
	{
		internal_callback(hsai, DSY_AUDIO_INTERNAL, offset);	
	}
	else
	{
		internal_callback(hsai, DSY_AUDIO_EXTERNAL, offset);
	}
//	for (size_t i = 0; i < audio_handle.block_size; i += 2)
//	{
//		audio_handle.in[i] = (float)((audio_handle.dma_buffer_rx[offset + i]))  / MAX_AMP;
//		audio_handle.in[i + 1] = (float)((audio_handle.dma_buffer_rx[offset + i + 1])) / MAX_AMP;
//	}
//	if (audio_handle.callback)
//	{
//		(audio_handle.callback)((float*)audio_handle.in,(float*)audio_handle.out,(size_t)audio_handle.block_size);
//	}
//	for (size_t i = 0; i < audio_handle.block_size; i+=2)
//	{
//		audio_handle.dma_buffer_tx[offset + i] = (int32_t)(audio_handle.out[i] * MAX_OUT_AMP);
//		audio_handle.dma_buffer_tx[offset + i + 1] = (int32_t)(audio_handle.out[i+1] * MAX_OUT_AMP);
//	}
#endif
}
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef* hsai)
{
	size_t offset = audio_handle.block_size;
#ifdef TEST_BYPASS
	//TransferComplete_CallBack_FS();
	test_bypass(offset);	
#else
	if (hsai->Instance == SAI1_Block_A)
	{
		internal_callback(hsai, DSY_AUDIO_INTERNAL, offset);	
	}
	else
	{
		internal_callback(hsai, DSY_AUDIO_EXTERNAL, offset);
	}
	//	for (size_t i = 0; i < audio_handle.block_size; i += 2)
	//	{
	//		audio_handle.in[i] = (float)((audio_handle.dma_buffer_rx[offset + i]) / MAX_AMP);
	//		audio_handle.in[i + 1] = (float)((audio_handle.dma_buffer_rx[offset + i + 1]) / MAX_AMP);
	//	}
	//	if (audio_handle.callback)
	//	{
	//		(audio_handle.callback)((float*)audio_handle.in,(float*)audio_handle.out,(size_t)audio_handle.block_size);
	//	}
	//	for (size_t i = 0; i < audio_handle.block_size; i+=2)
	//	{
	//		audio_handle.in[i] = (float)(audio_handle.dma_buffer_rx[offset + i]) / MAX_OUT_AMP;
	//		audio_handle.in[i + 1] = (float)(audio_handle.dma_buffer_rx[offset + i + 1]) / MAX_OUT_AMP;
	//		audio_handle.dma_buffer_tx[offset + i] = (int32_t)(audio_handle.out[i] * MAX_OUT_AMP);
	//		audio_handle.dma_buffer_tx[offset + i + 1] = (int32_t)(audio_handle.out[i+1] * MAX_OUT_AMP);
	//	}
#endif
}


#ifdef TEST_BYPASS
static void test_bypass(int32_t offset)
{
//	for (size_t i = 0; i < audio_handle.block_size; i++)
//	{
//		audio_handle.dma_buffer_tx[i + offset] = audio_handle.dma_buffer_rx[i + offset];
//	}
}
#endif