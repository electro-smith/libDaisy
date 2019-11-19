#include <stm32h7xx_hal.h>
#include "libdaisy.h"

static const uint32_t dsy_adc_channel_map[DSY_ADC_PIN_LAST] = 
{
		ADC_CHANNEL_3,
		ADC_CHANNEL_4,
		ADC_CHANNEL_5,
		ADC_CHANNEL_7,
		ADC_CHANNEL_10,
		ADC_CHANNEL_11,
		ADC_CHANNEL_15,
		ADC_CHANNEL_16,
		ADC_CHANNEL_17,
		ADC_CHANNEL_18,
		ADC_CHANNEL_19,
};
static const uint32_t dsy_adc_rank_map[] = 
{
		ADC_REGULAR_RANK_1,
		ADC_REGULAR_RANK_2,
		ADC_REGULAR_RANK_3,
		ADC_REGULAR_RANK_4,
		ADC_REGULAR_RANK_5,
		ADC_REGULAR_RANK_6,
		ADC_REGULAR_RANK_7,
		ADC_REGULAR_RANK_8,
		ADC_REGULAR_RANK_9,
		ADC_REGULAR_RANK_10,
		ADC_REGULAR_RANK_11,
		ADC_REGULAR_RANK_12,
		ADC_REGULAR_RANK_13,
		ADC_REGULAR_RANK_14,
		ADC_REGULAR_RANK_15,
		ADC_REGULAR_RANK_16,
};

#define DSY_ADC_MAX_CHANNELS DSY_ADC_PIN_LAST
#define DSY_ADC_MAX_RESOLUTION 65536.0f
typedef struct
{
	uint8_t  channels;
	uint16_t dma_buffer[DSY_ADC_MAX_CHANNELS];
	dsy_adc_handle_t* dsy_hadc;
} dsy_adc_t;

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

static dsy_adc_t dsy_adc;
/* ADC1 init function */
void dsy_adc_init(dsy_adc_handle_t *dsy_hadc)
{
	ADC_MultiModeTypeDef   multimode = {0};
	ADC_ChannelConfTypeDef sConfig   = {0};
	//dsy_adc_handle.board	= board;
	dsy_adc.dsy_hadc = dsy_hadc;
	dsy_adc.channels = dsy_hadc->channels; 
	for(uint8_t i = 0; i < DSY_ADC_MAX_CHANNELS; i++)
	{
		dsy_adc.dma_buffer[i] = 0;
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
	hadc1.Init.NbrOfConversion			= dsy_adc.channels;
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
  // Configure Shared settings for all channels.
	sConfig.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;
	sConfig.SingleDiff   = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset		 = 0;
	for(uint8_t i = 0; i < dsy_adc.channels; i++) 
	{
		sConfig.Channel
			= dsy_adc_channel_map[dsy_adc.dsy_hadc->active_channels[i]];
		sConfig.Rank = dsy_adc_rank_map[i];
		//= dsy_adc_rank_map[dsy_adc.dsy_hadc->active_channels[i]];
		if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) 
		{
			// Error_Handler();
		}
	}
}
void dsy_adc_start(uint32_t* buff)
{
	HAL_ADC_Start_DMA(
		&hadc1, (uint32_t*)&dsy_adc.dma_buffer, dsy_adc.channels);
}
void dsy_adc_stop()
{
	HAL_ADC_Stop_DMA(&hadc1);
}
uint16_t dsy_adc_get(uint8_t chn) 
{
	return dsy_adc.dma_buffer[chn < DSY_ADC_MAX_CHANNELS ? chn : 0];
}
float	dsy_adc_get_float(uint8_t chn) 
{
	return (float)dsy_adc.dma_buffer[chn < DSY_ADC_MAX_CHANNELS ? chn : 0] / DSY_ADC_MAX_RESOLUTION;
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(adcHandle->Instance == ADC1)
	{
		/* ADC1 clock enable */
		__HAL_RCC_ADC12_CLK_ENABLE();

		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		// Only initialize the pins being used.
		GPIO_TypeDef* port;
		dsy_gpio_pin* pin_config;
		uint8_t*	  chn_list;
		pin_config = dsy_adc.dsy_hadc->pin_config;
		chn_list   = dsy_adc.dsy_hadc->active_channels;
		for(uint8_t i = 0; i < dsy_adc.channels; i++) 
		{
			port = (GPIO_TypeDef*)gpio_hal_port_map[pin_config[chn_list[i]].port];
			GPIO_InitStruct.Pin = gpio_hal_pin_map[pin_config[chn_list[i]].pin];
			GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			HAL_GPIO_Init(port, &GPIO_InitStruct);
		}

		/* ADC1 DMA Init */
		/* ADC1 Init */
		hdma_adc1.Instance				   = DMA1_Stream2;
		hdma_adc1.Init.Request			   = DMA_REQUEST_ADC1;
		hdma_adc1.Init.Direction		   = DMA_PERIPH_TO_MEMORY;
		hdma_adc1.Init.PeriphInc		   = DMA_PINC_DISABLE;
		hdma_adc1.Init.MemInc			   = DMA_MINC_ENABLE;
		hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_adc1.Init.MemDataAlignment	   = DMA_MDATAALIGN_HALFWORD;
		hdma_adc1.Init.Mode				   = DMA_CIRCULAR;
		hdma_adc1.Init.Priority			   = DMA_PRIORITY_LOW;
		hdma_adc1.Init.FIFOMode			   = DMA_FIFOMODE_DISABLE;
		if(HAL_DMA_Init(&hdma_adc1) != HAL_OK)
		{
			//Error_Handler();
		}

		__HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc1);
	}
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{
	if(adcHandle->Instance == ADC1)
	{
		// Peripheral clock disable
		__HAL_RCC_ADC12_CLK_DISABLE();
		GPIO_TypeDef* port;
		dsy_gpio_pin* pin_config;
		uint8_t*	  chn_list;
		uint16_t	  pin;
		pin_config = dsy_adc.dsy_hadc->pin_config;
		chn_list   = dsy_adc.dsy_hadc->active_channels;
		for(uint8_t i = 0; i < dsy_adc.channels; i++) 
		{
			port = (GPIO_TypeDef*)gpio_hal_port_map[pin_config[chn_list[i]].port];
			pin = gpio_hal_pin_map[pin_config[chn_list[i]].pin];
			HAL_GPIO_DeInit(port, pin);
		}
		HAL_DMA_DeInit(adcHandle->DMA_Handle);
	}
}

void DMA1_Stream2_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_adc1);
}
