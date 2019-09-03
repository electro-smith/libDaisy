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
#define MAX_AMP 65535.0f
#define MAX_OUT_AMP 65535.0f
#define MAX_IN_AMP 65535.0f
// Define/Declare global audio structure. 
typedef struct
{
    audio_callback callback;
    int32_t dma_buffer_rx[DMA_BUFFER_SIZE];
    int32_t dma_buffer_tx[DMA_BUFFER_SIZE];
    float in[BLOCK_SIZE];
    float out[BLOCK_SIZE];
    size_t block_size;
	size_t offset;
}et_audio_t;
static et_audio_t audio_handle;

extern DMA_HandleTypeDef hdma_sai1_a;
extern DMA_HandleTypeDef hdma_sai1_b;

void DMA1_Stream0_IRQHandler(void)
{
	/* USER CODE BEGIN DMA1_Stream0_IRQn 0 */

	/* USER CODE END DMA1_Stream0_IRQn 0 */
	HAL_DMA_IRQHandler(&hdma_sai1_a);
	/* USER CODE BEGIN DMA1_Stream0_IRQn 1 */

	/* USER CODE END DMA1_Stream0_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream1 global interrupt.
  */
void DMA1_Stream1_IRQHandler(void)
{
	/* USER CODE BEGIN DMA1_Stream1_IRQn 0 */

	/* USER CODE END DMA1_Stream1_IRQn 0 */
	HAL_DMA_IRQHandler(&hdma_sai1_b);
	/* USER CODE BEGIN DMA1_Stream1_IRQn 1 */

	/* USER CODE END DMA1_Stream1_IRQn 1 */
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

void dsy_audio_init(uint8_t board)
{
	// For now expecting external MX_SAI_Init() is being run in main.c
	MX_DMA_Init();
	MX_SAI1_Init();
	MX_SAI2_Init();
	MX_I2C2_Init();
	if (board == DSY_SYS_BOARD_DAISY_SEED)
	{
		sa_codec_init(1, 48000.0f);
	}
	else
	{
		codec_pcm3060_init(&hi2c2);
	}
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
	#ifdef __USBD_AUDIO_IF_H__
	audio_start(); // start audio callbacks, and then we'll see if it works or not...
	#endif
}

void dsy_audio_set_callback(audio_callback cb)
{
    audio_handle.callback = cb;
}

void dsy_audio_start()
{
//    HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)audio_handle.dma_buffer_rx, DMA_BUFFER_SIZE);
//    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audio_handle.dma_buffer_tx, DMA_BUFFER_SIZE);
    HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)audio_handle.dma_buffer_rx, DMA_BUFFER_SIZE);
    HAL_SAI_Transmit_DMA(&hsai_BlockB1, (uint8_t *)audio_handle.dma_buffer_tx, DMA_BUFFER_SIZE);
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

void audio_stop()
{
    HAL_SAI_DMAStop(&hsai_BlockA1);
    HAL_SAI_DMAStop(&hsai_BlockB1);
}


#ifdef TEST_BYPASS
static void test_bypass(int32_t offset);
#endif
// DMA Callbacks
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef* hsai)
{
	size_t offset = 0;
#ifdef TEST_BYPASS
		//HalfTransfer_CallBack_FS();
	test_bypass(offset);
#else
	for (size_t i = 0; i < audio_handle.block_size; i += 2)
	{
		audio_handle.in[i] = (float)((audio_handle.dma_buffer_rx[offset + i]))  / MAX_AMP;
		audio_handle.in[i + 1] = (float)((audio_handle.dma_buffer_rx[offset + i + 1])) / MAX_AMP;
	}
	if (audio_handle.callback)
	{
		(audio_handle.callback)((float*)audio_handle.in,(float*)audio_handle.out,(size_t)audio_handle.block_size);
	}
	for (size_t i = 0; i < audio_handle.block_size; i+=2)
	{
		audio_handle.dma_buffer_tx[offset + i] = (int32_t)(audio_handle.out[i] * MAX_OUT_AMP);
		audio_handle.dma_buffer_tx[offset + i + 1] = (int32_t)(audio_handle.out[i+1] * MAX_OUT_AMP);
	}
#endif
}
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef* hsai)
{
	size_t offset = audio_handle.block_size;
#ifdef TEST_BYPASS
	//TransferComplete_CallBack_FS();
	test_bypass(offset);	
#else
	for (size_t i = 0; i < audio_handle.block_size; i += 2)
	{
		audio_handle.in[i] = (float)((audio_handle.dma_buffer_rx[offset + i]) / MAX_AMP);
		audio_handle.in[i + 1] = (float)((audio_handle.dma_buffer_rx[offset + i + 1]) / MAX_AMP);
	}
	if (audio_handle.callback)
	{
		(audio_handle.callback)((float*)audio_handle.in,(float*)audio_handle.out,(size_t)audio_handle.block_size);
	}
	for (size_t i = 0; i < audio_handle.block_size; i+=2)
	{
		audio_handle.in[i] = (float)(audio_handle.dma_buffer_rx[offset + i]) / MAX_OUT_AMP;
		audio_handle.in[i + 1] = (float)(audio_handle.dma_buffer_rx[offset + i + 1]) / MAX_OUT_AMP;
		audio_handle.dma_buffer_tx[offset + i] = (int32_t)(audio_handle.out[i] * MAX_OUT_AMP);
		audio_handle.dma_buffer_tx[offset + i + 1] = (int32_t)(audio_handle.out[i+1] * MAX_OUT_AMP);
	}
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