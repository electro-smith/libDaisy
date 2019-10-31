#include "dsy_dac.h"
#include "stm32h7xx_hal.h"

typedef struct
{
	DAC_HandleTypeDef hdac1; // ST HAL DAC handle
	dsy_dac_handle_t *dsy_hdac;
	uint8_t			  initialized;
	dsy_dac_channel   active_channels;
} dsy_dac_t;

static dsy_dac_t dac;

void dsy_dac_init(dsy_dac_handle_t *dsy_hdac, dsy_dac_channel channel) 
{
	DAC_ChannelConfTypeDef dac_config;
	dac.dsy_hdac = dsy_hdac;
	if(!dac.initialized)
	{
		// Init the actual peripheral
		dac.hdac1.Instance = DAC1;
		HAL_DAC_Init(&dac.hdac1);
		dac.initialized = 1;
	}
	// Setup common config
	dac_config.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
	dac_config.DAC_Trigger
		= DAC_TRIGGER_SOFTWARE; // TODO: Set to timer base
	dac_config.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
	dac_config.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
	dac_config.DAC_UserTrimming			   = DAC_TRIMMING_FACTORY;
	if(channel == DSY_DAC_CHN1 || channel == DSY_DAC_CHN_BOTH) 
	{
		// Init Channel 1
		if(HAL_DAC_ConfigChannel(&dac.hdac1, &dac_config, DAC_CHANNEL_1)
		   != HAL_OK)
		{
			// Fail
		} 
	}
	if(channel == DSY_DAC_CHN2 || channel == DSY_DAC_CHN_BOTH) 
	{
		// Init Channel 2
		if(HAL_DAC_ConfigChannel(&dac.hdac1, &dac_config, DAC_CHANNEL_2)
		   != HAL_OK)
		{
			// Fail
		} 
	}
	dac.active_channels = channel;
}
 
void dsy_dac_start(dsy_dac_channel channel) 
{
	switch(channel)
	{
		case DSY_DAC_CHN1: HAL_DAC_Start(&dac.hdac1, DAC_CHANNEL_1); break;
		case DSY_DAC_CHN2: HAL_DAC_Start(&dac.hdac1, DAC_CHANNEL_2); break;
		case DSY_DAC_CHN_BOTH: 
			HAL_DAC_Start(&dac.hdac1, DAC_CHANNEL_1); 
			HAL_DAC_Start(&dac.hdac1, DAC_CHANNEL_2); 
			break;
		default: break;
	}
}
void dsy_dac_write(dsy_dac_channel channel, uint16_t val)
{
	uint8_t bitdepth;
	if(dac.dsy_hdac->bitdepth == DSY_DAC_BITS_8) 
	{
		bitdepth = DAC_ALIGN_8B_R;
	}
	else
	{
		bitdepth = DAC_ALIGN_12B_R;
	}
	switch(channel)
	{
		case DSY_DAC_CHN1: HAL_DAC_SetValue(&dac.hdac1, DAC_CHANNEL_1, bitdepth, val); break;
		case DSY_DAC_CHN2: HAL_DAC_SetValue(&dac.hdac1, DAC_CHANNEL_2, bitdepth, val); break;
		case DSY_DAC_CHN_BOTH:
			HAL_DAC_SetValue(&dac.hdac1, DAC_CHANNEL_1, bitdepth, val);
			HAL_DAC_SetValue(&dac.hdac1, DAC_CHANNEL_2, bitdepth, val);
			break;
		default: break;
	}
	// If there's a timer running, this isn't necessary.
	if(dac.dsy_hdac->mode == DSY_DAC_MODE_POLLING) 
	{
		dsy_dac_start(channel);
	}
}

void HAL_DAC_MspInit(DAC_HandleTypeDef *hdac) 
{
	GPIO_InitTypeDef ginit;
	GPIO_TypeDef *	port;
	if(hdac->Instance == DAC1) 
	{
		__HAL_RCC_DAC12_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		ginit.Mode = GPIO_MODE_ANALOG;
		ginit.Pull = GPIO_NOPULL;
		if(dac.active_channels == DSY_DAC_CHN1
		   || dac.active_channels == DSY_DAC_CHN_BOTH)
		{
			port = (GPIO_TypeDef
						*)(gpio_hal_port_map
							   [dac.dsy_hdac->pin_config[DSY_DAC_CHN1].port]);
			ginit.Pin
				= gpio_hal_pin_map[dac.dsy_hdac->pin_config[DSY_DAC_CHN1].pin];
			HAL_GPIO_Init(port, &ginit);
		}
		if(dac.active_channels == DSY_DAC_CHN2
		   || dac.active_channels == DSY_DAC_CHN_BOTH)
		{
			port = (GPIO_TypeDef
						*)(gpio_hal_port_map
							   [dac.dsy_hdac->pin_config[DSY_DAC_CHN2].port]);
			ginit.Pin
				= gpio_hal_pin_map[dac.dsy_hdac->pin_config[DSY_DAC_CHN2].pin];
			HAL_GPIO_Init(port, &ginit);
		}
	}
}

void HAL_DAC_MspDeInit(DAC_HandleTypeDef *hdac) 
{
	GPIO_TypeDef *	port;
	uint16_t		pin;
	if(hdac->Instance == DAC1)
	{
		__HAL_RCC_DAC12_CLK_DISABLE();
		if(dac.active_channels == DSY_DAC_CHN1
		   || dac.active_channels == DSY_DAC_CHN_BOTH)
		{
			port = (GPIO_TypeDef
						*)(gpio_hal_port_map
							   [dac.dsy_hdac->pin_config[DSY_DAC_CHN1].port]);
			pin
				= gpio_hal_pin_map[dac.dsy_hdac->pin_config[DSY_DAC_CHN1].pin];
			HAL_GPIO_DeInit(port, pin);
		}
		if(dac.active_channels == DSY_DAC_CHN2
		   || dac.active_channels == DSY_DAC_CHN_BOTH)
		{
			port = (GPIO_TypeDef
						*)(gpio_hal_port_map
							   [dac.dsy_hdac->pin_config[DSY_DAC_CHN2].port]);
			pin
				= gpio_hal_pin_map[dac.dsy_hdac->pin_config[DSY_DAC_CHN2].pin];
			HAL_GPIO_DeInit(port, pin);
		}
	}
}
