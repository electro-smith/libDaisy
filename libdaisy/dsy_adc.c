#ifdef __cplusplus
extern "C" {
#endif

#include "libdaisy.h"
#include <stm32h7xx_hal.h>


#define DSY_ADC_MAX_CHANNELS 8
#define DSY_ADC_MAX_RESOLUTION 65536.0f
typedef struct
{
	uint8_t  board;
	uint8_t  channels;
	uint16_t dma_buffer[DSY_ADC_MAX_CHANNELS];
} dsy_adc_handle_t;

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

static dsy_adc_handle_t dsy_adc_handle;
/* ADC1 init function */
void dsy_adc_init(uint8_t board)
{
	ADC_MultiModeTypeDef   multimode = {0};
	ADC_ChannelConfTypeDef sConfig   = {0};

	dsy_adc_handle.board	= board;
	dsy_adc_handle.channels = 8; // fixed for now.
	for(uint8_t i = 0; i < DSY_ADC_MAX_CHANNELS; i++)
	{
		dsy_adc_handle.dma_buffer[i] = 0;
	}
	/** Common config 
  */
	hadc1.Instance						= ADC1;
	hadc1.Init.ClockPrescaler			= ADC_CLOCK_ASYNC_DIV128;
	hadc1.Init.Resolution				= ADC_RESOLUTION_16B;
	hadc1.Init.ScanConvMode				= ADC_SCAN_ENABLE;
	hadc1.Init.EOCSelection				= ADC_EOC_SEQ_CONV;
	hadc1.Init.LowPowerAutoWait			= DISABLE;
	hadc1.Init.ContinuousConvMode		= ENABLE;
	hadc1.Init.NbrOfConversion			= 8;
	hadc1.Init.DiscontinuousConvMode	= DISABLE;
	hadc1.Init.ExternalTrigConv			= ADC_SOFTWARE_START;
	hadc1.Init.ExternalTrigConvEdge		= ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
	hadc1.Init.Overrun					= ADC_OVR_DATA_PRESERVED;
	hadc1.Init.LeftBitShift				= ADC_LEFTBITSHIFT_NONE;
	hadc1.Init.OversamplingMode			= DISABLE;
	if(HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		//Error_Handler();
	}
	/** Configure the ADC multi-mode 
  */
	multimode.Mode = ADC_MODE_INDEPENDENT;
	if(HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
	{
		//Error_Handler();
	}
	/** Configure Regular Channel 
  */
	sConfig.Channel		 = ADC_CHANNEL_3;
	sConfig.Rank		 = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;
	sConfig.SingleDiff   = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset		 = 0;
	if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		//Error_Handler();
	}
	/** Configure Regular Channel 
  */
	sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Rank	= ADC_REGULAR_RANK_2;
	if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		//Error_Handler();
	}
	/** Configure Regular Channel 
  */
	sConfig.Channel = ADC_CHANNEL_7;
	sConfig.Rank	= ADC_REGULAR_RANK_3;
	if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		//Error_Handler();
	}
	/** Configure Regular Channel 
  */
	sConfig.Channel = ADC_CHANNEL_10;
	sConfig.Rank	= ADC_REGULAR_RANK_4;
	if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		//Error_Handler();
	}
	/** Configure Regular Channel 
  */
	sConfig.Channel = ADC_CHANNEL_11;
	sConfig.Rank	= ADC_REGULAR_RANK_5;
	if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		//Error_Handler();
	}
	/** Configure Regular Channel 
  */
	sConfig.Channel = ADC_CHANNEL_14;
	sConfig.Rank	= ADC_REGULAR_RANK_6;
	if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		//Error_Handler();
	}
	/** Configure Regular Channel 
  */
	sConfig.Channel = ADC_CHANNEL_15;
	sConfig.Rank	= ADC_REGULAR_RANK_7;
	if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		//Error_Handler();
	}
	/** Configure Regular Channel 
  */
	sConfig.Channel = ADC_CHANNEL_DAC1CH2_ADC2;
	sConfig.Rank	= ADC_REGULAR_RANK_8;
	if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		//Error_Handler();
	}
}
void dsy_adc_start(uint32_t* buff)
{
	HAL_ADC_Start_DMA(
		&hadc1, (uint32_t*)&dsy_adc_handle.dma_buffer, dsy_adc_handle.channels);
}
void dsy_adc_stop()
{
	HAL_ADC_Stop_DMA(&hadc1);
}
uint16_t dsy_adc_get(uint8_t chn) 
{
	return dsy_adc_handle.dma_buffer[chn < DSY_ADC_MAX_CHANNELS ? chn : 0];
}
float	dsy_adc_get_float(uint8_t chn) 
{
	return (float)dsy_adc_handle.dma_buffer[chn < DSY_ADC_MAX_CHANNELS ? chn : 0] / DSY_ADC_MAX_RESOLUTION;
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(adcHandle->Instance == ADC1)
	{
		/* USER CODE BEGIN ADC1_MspInit 0 */

		/* USER CODE END ADC1_MspInit 0 */
		/* ADC1 clock enable */
		__HAL_RCC_ADC12_CLK_ENABLE();

		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**ADC1 GPIO Configuration    
    PC0     ------> ADC1_INP10
    PC1     ------> ADC1_INP11
    PA1     ------> ADC1_INP17
    PA2     ------> ADC1_INP14
    PA6     ------> ADC1_INP3
    PA3     ------> ADC1_INP15
    PA7     ------> ADC1_INP7 
    */
		GPIO_InitStruct.Pin  = GPIO_PIN_0 | GPIO_PIN_1;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct.Pin
			= GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_3 | GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* ADC1 DMA Init */
		/* ADC1 Init */
		hdma_adc1.Instance				   = DMA1_Stream2;
		hdma_adc1.Init.Request			   = DMA_REQUEST_ADC1;
		hdma_adc1.Init.Direction		   = DMA_PERIPH_TO_MEMORY;
		hdma_adc1.Init.PeriphInc		   = DMA_PINC_DISABLE;
		hdma_adc1.Init.MemInc			   = DMA_MINC_ENABLE;
		hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_adc1.Init.MemDataAlignment	= DMA_MDATAALIGN_HALFWORD;
		hdma_adc1.Init.Mode				   = DMA_CIRCULAR;
		hdma_adc1.Init.Priority			   = DMA_PRIORITY_LOW;
		hdma_adc1.Init.FIFOMode			   = DMA_FIFOMODE_DISABLE;
		if(HAL_DMA_Init(&hdma_adc1) != HAL_OK)
		{
			//Error_Handler();
		}

		__HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc1);

		/* USER CODE BEGIN ADC1_MspInit 1 */

		/* USER CODE END ADC1_MspInit 1 */
	}
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{
	if(adcHandle->Instance == ADC1)
	{
		// USER CODE BEGIN ADC1_MspDeInit 0 

		// USER CODE END ADC1_MspDeInit 0 
		// Peripheral clock disable
		__HAL_RCC_ADC12_CLK_DISABLE();

//		ADC1 GPIO Configuration    
//    PC0     ------> ADC1_INP10
//    PC1     ------> ADC1_INP11
//    PA1     ------> ADC1_INP17
//    PA2     ------> ADC1_INP14
//    PA6     ------> ADC1_INP3
//    PA3     ------> ADC1_INP15
//    PA7     ------> ADC1_INP7 
		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0|GPIO_PIN_1);

		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_6|GPIO_PIN_3 
							  |GPIO_PIN_7);
    // ADC1 DMA DeInit 
		HAL_DMA_DeInit(adcHandle->DMA_Handle);
	}
}

void DMA1_Stream2_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_adc1);
}

#ifdef __cplusplus
}
#endif
