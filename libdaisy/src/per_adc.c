#include <stm32h7xx_hal.h>
#include "per_adc.h"

static const uint32_t dsy_adc_channel_map[DSY_ADC_PIN_LAST] = {
	ADC_CHANNEL_3,
	ADC_CHANNEL_4,
	ADC_CHANNEL_5,
	ADC_CHANNEL_7,
	ADC_CHANNEL_8,
	ADC_CHANNEL_9,
	ADC_CHANNEL_10,
	ADC_CHANNEL_11,
	ADC_CHANNEL_15,
	ADC_CHANNEL_16,
	ADC_CHANNEL_17,
	ADC_CHANNEL_18,
	ADC_CHANNEL_19,
};
static const uint32_t dsy_adc_rank_map[] = {
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
#define DSY_ADC_MAX_MUX_CHANNELS 8
#define DSY_ADC_MAX_RESOLUTION 65536.0f

static uint16_t __attribute__((section(".sram1_bss"))) adc1_dma_buffer[DSY_ADC_MAX_CHANNELS];
typedef struct
{
	uint8_t			  channels, mux_channels[DSY_ADC_MAX_CHANNELS];
	uint16_t*		  dma_buffer;
	uint16_t		  mux_cache[DSY_ADC_MAX_CHANNELS][DSY_ADC_MAX_MUX_CHANNELS];
	uint16_t		  mux_index[DSY_ADC_MAX_CHANNELS]; // 0->mux_channels per ADC channel
	dsy_adc_handle* dsy_hadc;
} dsy_adc;

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
static dsy_adc adc;

static void write_mux_value(uint8_t chn, uint8_t idx);

/* ADC1 init function */
void dsy_adc_init(dsy_adc_handle* dsy_hadc)
{
	ADC_MultiModeTypeDef   multimode = {0};
	ADC_ChannelConfTypeDef sConfig   = {0};
	//dsy_adc_handle.board	= board;
	adc.dsy_hadc = dsy_hadc;
	adc.channels = dsy_hadc->channels;
	adc.dma_buffer = adc1_dma_buffer;
	//	dsy_adc.mux_channels = dsy_hadc->mux_channels;
	for(uint8_t i = 0; i < DSY_ADC_MAX_CHANNELS; i++)
	{
		adc.dma_buffer[i] = 0;
		adc.mux_channels[i] = dsy_hadc->mux_channels[i];
	}
	/** Common config 
  */
	hadc1.Instance						= ADC1;
	hadc1.Init.ClockPrescaler			= ADC_CLOCK_ASYNC_DIV2;
	hadc1.Init.Resolution				= ADC_RESOLUTION_16B;
	hadc1.Init.ScanConvMode				= ADC_SCAN_ENABLE;
	hadc1.Init.EOCSelection				= ADC_EOC_SEQ_CONV;
	hadc1.Init.LowPowerAutoWait			= DISABLE;
	hadc1.Init.ContinuousConvMode		= ENABLE;
	hadc1.Init.NbrOfConversion			= adc.channels;
	hadc1.Init.DiscontinuousConvMode	= DISABLE;
	hadc1.Init.ExternalTrigConv			= ADC_SOFTWARE_START;
	hadc1.Init.ExternalTrigConvEdge		= ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
	hadc1.Init.Overrun					= ADC_OVR_DATA_PRESERVED;
	hadc1.Init.LeftBitShift				= ADC_LEFTBITSHIFT_NONE;
	if(dsy_hadc->oversampling)
	{
		hadc1.Init.OversamplingMode = ENABLE;
		hadc1.Init.Oversampling.OversamplingStopReset
			= ADC_REGOVERSAMPLING_CONTINUED_MODE;
		hadc1.Init.Oversampling.TriggeredMode
			= ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
	}
	else
	{
		hadc1.Init.OversamplingMode = DISABLE;
	}
	switch(dsy_hadc->oversampling)
	{
		case DSY_ADC_OVS_4:
			hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_2;
			hadc1.Init.Oversampling.Ratio		  = 3;
			break;
		case DSY_ADC_OVS_8:
			hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_3;
			hadc1.Init.Oversampling.Ratio		  = 7;
			break;
		case DSY_ADC_OVS_16:
			hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_4;
			hadc1.Init.Oversampling.Ratio		  = 15;
			break;
		case DSY_ADC_OVS_32:
			hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_5;
			hadc1.Init.Oversampling.Ratio		  = 31;
			break;
		case DSY_ADC_OVS_64:
			hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_6;
			hadc1.Init.Oversampling.Ratio		  = 63;
			break;
		case DSY_ADC_OVS_128:
			hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_7;
			hadc1.Init.Oversampling.Ratio		  = 127;
			break;
		case DSY_ADC_OVS_256:
			hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_8;
			hadc1.Init.Oversampling.Ratio		  = 255;
			break;
		case DSY_ADC_OVS_512:
			hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_9;
			hadc1.Init.Oversampling.Ratio		  = 511;
			break;
		case DSY_ADC_OVS_1024:
			hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_10;
			hadc1.Init.Oversampling.Ratio		  = 1023;
			break;
		default: break;
	}

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
	sConfig.SamplingTime = ADC_SAMPLETIME_8CYCLES_5;
	sConfig.SingleDiff   = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset		 = 0;
	for(uint8_t i = 0; i < adc.channels; i++)
	{
		sConfig.Channel
			= dsy_adc_channel_map[adc.dsy_hadc->active_channels[i]];
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
    HAL_ADCEx_Calibration_Start(
        &hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc.dma_buffer, adc.channels);
}
void dsy_adc_stop()
{
	HAL_ADC_Stop_DMA(&hadc1);
}
uint16_t dsy_adc_get(uint8_t chn)
{
	return adc.dma_buffer[chn < DSY_ADC_MAX_CHANNELS ? chn : 0];
}
uint16_t *dsy_adc_get_rawptr(uint8_t chn)
{
	return &adc.dma_buffer[chn < DSY_ADC_MAX_CHANNELS ? chn : 0];
}

float dsy_adc_get_float(uint8_t chn)
{
	return (float)adc.dma_buffer[chn < DSY_ADC_MAX_CHANNELS ? chn : 0]
		   / DSY_ADC_MAX_RESOLUTION;
}

uint16_t dsy_adc_get_mux(uint8_t chn, uint8_t idx) 
{
	return adc.mux_cache[chn < DSY_ADC_MAX_CHANNELS ? chn : 0][idx];
}

uint16_t *dsy_adc_get_mux_rawptr(uint8_t chn, uint8_t idx)
{
	return &adc.mux_cache[chn < DSY_ADC_MAX_CHANNELS ? chn : 0][idx];
}

float dsy_adc_get_mux_float(uint8_t chn, uint8_t idx) 
{
	return (float)adc.mux_cache[chn < DSY_ADC_MAX_CHANNELS ? chn : 0][idx]
		   / DSY_ADC_MAX_RESOLUTION;
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(adcHandle->Instance == ADC1)
	{
		/* ADC1 clock enable */
		__HAL_RCC_ADC12_CLK_ENABLE();

		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		// Only initialize the pins being used.
		GPIO_TypeDef* port;
		dsy_gpio_pin* pin_config;
		uint8_t*	  chn_list;
		pin_config = adc.dsy_hadc->pin_config;
		chn_list   = adc.dsy_hadc->active_channels;
		for(uint8_t i = 0; i < adc.channels; i++)
		{
			port = (GPIO_TypeDef*)
				gpio_hal_port_map[pin_config[chn_list[i]].port];
			GPIO_InitStruct.Pin = gpio_hal_pin_map[pin_config[chn_list[i]].pin];
			GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			HAL_GPIO_Init(port, &GPIO_InitStruct);
			// Initialize Mux Pins if necessary
			if(adc.mux_channels[i] > 0) 
			{
				pin_config = adc.dsy_hadc->mux_pin_config[i];
				for(uint16_t j = 0; j < MUX_SEL_LAST; j++)
				{
					port = (GPIO_TypeDef*)gpio_hal_port_map[pin_config[j].port];
					GPIO_InitStruct.Pin = gpio_hal_pin_map[pin_config[j].pin];
					GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
					GPIO_InitStruct.Pull = GPIO_NOPULL;
					GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
					HAL_GPIO_Init(port, &GPIO_InitStruct);
				}
			}
		}

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
		pin_config = adc.dsy_hadc->pin_config;
		chn_list   = adc.dsy_hadc->active_channels;
		for(uint8_t i = 0; i < adc.channels; i++)
		{
			port = (GPIO_TypeDef*)
				gpio_hal_port_map[pin_config[chn_list[i]].port];
			pin = gpio_hal_pin_map[pin_config[chn_list[i]].pin];
			HAL_GPIO_DeInit(port, pin);
			// TODO: Add Mux DeInit
		}
		HAL_DMA_DeInit(adcHandle->DMA_Handle);
	}
}

void DMA1_Stream2_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_adc1);
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) 
{
	if(hadc->Instance == ADC1) 
	{
		// Handle Externally Multiplexed Pins
		for(uint16_t i = 0; i < adc.channels; i++)
		{
			dsy_adc_pin t = (dsy_adc_pin)adc.dsy_hadc->active_channels[i];
			uint8_t current_position = adc.mux_index[i];
			if(adc.mux_channels[t] > 0)
			{
				// Capture current value to mux_cache
				adc.mux_cache[i][current_position] = adc.dma_buffer[i];
				// Update Mux Position, and write GPIO
				adc.mux_index[i] += 1;
				if(adc.mux_index[i] > adc.mux_channels[t])
				{
					adc.mux_index[i] = 0;
				}
				write_mux_value(t, adc.mux_index[i]);
			}
		}
	}
}

static void write_mux_value(uint8_t chn, uint8_t idx) 
{
	GPIO_PinState b0, b1, b2;
	dsy_gpio_pin *pincfg = adc.dsy_hadc->mux_pin_config[chn];
	b0					 = (GPIO_PinState)((idx & 0x01) > 0 ? 1 : 0);
	b1					 = (GPIO_PinState)((idx & 0x02) > 0 ? 1 : 0);
	b2					 = (GPIO_PinState)((idx & 0x04) > 0 ? 1 : 0);
	HAL_GPIO_WritePin((GPIO_TypeDef*)gpio_hal_port_map[pincfg[0].port], gpio_hal_pin_map[pincfg[0].pin], b0);
	HAL_GPIO_WritePin((GPIO_TypeDef*)gpio_hal_port_map[pincfg[1].port], gpio_hal_pin_map[pincfg[1].pin], b1);
	HAL_GPIO_WritePin((GPIO_TypeDef*)gpio_hal_port_map[pincfg[2].port], gpio_hal_pin_map[pincfg[2].pin], b2);
}
