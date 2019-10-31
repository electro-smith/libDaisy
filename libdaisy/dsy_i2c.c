#include "dsy_i2c.h"

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;
I2C_HandleTypeDef hi2c4;

// TODO: This is global, and the board gets set for each init. 
// Its a bit redundant, but I'm just trying to validate some hardware
//   without breaking all the other boards.
static dsy_i2c_handle_t i2c1_handler;
static dsy_i2c_handle_t i2c2_handler;
static dsy_i2c_handle_t i2c3_handler;
static dsy_i2c_handle_t i2c4_handler;


static void init_i2c_pins(dsy_i2c_handle_t* hi2c);
static void deinit_i2c_pins(dsy_i2c_handle_t* hi2c);

void dsy_i2c_init(dsy_i2c_handle_t *dsy_hi2c) 
{
	I2C_HandleTypeDef *hal_hi2c;
	switch(dsy_hi2c->periph)
	{
		case DSY_I2C_PERIPH_1: 
			i2c1_handler = *dsy_hi2c;
			hal_hi2c	 = &hi2c1;
			hal_hi2c->Instance = I2C1;
			break;
		case DSY_I2C_PERIPH_2: 
			i2c2_handler = *dsy_hi2c;
			hal_hi2c	 = &hi2c2;
			hal_hi2c->Instance = I2C2;
			break;
		case DSY_I2C_PERIPH_3: 
			i2c3_handler = *dsy_hi2c;
			hal_hi2c	 = &hi2c3;
			hal_hi2c->Instance = I2C3;
			break;
		case DSY_I2C_PERIPH_4:
			i2c4_handler = *dsy_hi2c;
			hal_hi2c	 = &hi2c4;
			hal_hi2c->Instance = I2C4;
			break;
		default: 
			i2c1_handler = *dsy_hi2c;
			hal_hi2c	 = &hi2c1;
			hal_hi2c->Instance = I2C1;
			break;
	}
	// Set Generic Parameters
	hal_hi2c->Init.Timing = 0x00C0EAFF;
	hal_hi2c->Init.OwnAddress1 = 0;
	hal_hi2c->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hal_hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hal_hi2c->Init.OwnAddress2 = 0;
	hal_hi2c->Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hal_hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hal_hi2c->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(hal_hi2c) != HAL_OK)
	{
	//Error_Handler();
	}
	if (HAL_I2CEx_ConfigAnalogFilter(hal_hi2c, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
	//Error_Handler();
	}
	if (HAL_I2CEx_ConfigDigitalFilter(hal_hi2c, 0) != HAL_OK)
	{
	//Error_Handler();
	}
}

I2C_HandleTypeDef* dsy_i2c_hal_handle(dsy_i2c_handle_t* dsy_hi2c) 
{
	switch(dsy_hi2c->periph)
	{
		case DSY_I2C_PERIPH_1: return &hi2c1;
		case DSY_I2C_PERIPH_2: return &hi2c2;
		case DSY_I2C_PERIPH_3: return &hi2c3;
		case DSY_I2C_PERIPH_4: return &hi2c4;
		default: return NULL;
	}
}

// TODO: Fix the DeInits 
void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

	if(i2cHandle->Instance == I2C1)
	{
	  __HAL_RCC_GPIOB_CLK_ENABLE();
	  init_i2c_pins(&i2c1_handler);
	  __HAL_RCC_I2C1_CLK_ENABLE();
	}
	else if(i2cHandle->Instance == I2C2)
	{
	  __HAL_RCC_GPIOH_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();
	  init_i2c_pins(&i2c2_handler);
	  __HAL_RCC_I2C2_CLK_ENABLE();
	}
	else if(i2cHandle->Instance == I2C3)
	{
		// Enable RCC GPIO CLK for necessary ports.
	  init_i2c_pins(&i2c3_handler);
	  __HAL_RCC_I2C3_CLK_ENABLE();
	}
	else if(i2cHandle->Instance == I2C4)
	{
	  __HAL_RCC_GPIOB_CLK_ENABLE();
	  init_i2c_pins(&i2c4_handler);
	  __HAL_RCC_I2C4_CLK_ENABLE();
	}
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{
	if(i2cHandle->Instance == I2C1)
	{
	  __HAL_RCC_I2C1_CLK_DISABLE();
	  deinit_i2c_pins(&i2c1_handler);
	}
	else if(i2cHandle->Instance == I2C2)
	{
	  __HAL_RCC_I2C2_CLK_DISABLE();
	  deinit_i2c_pins(&i2c2_handler);
	}
	else if(i2cHandle->Instance == I2C3)
	{
		// Enable RCC GPIO CLK for necessary ports.
	  __HAL_RCC_I2C3_CLK_DISABLE();
	  deinit_i2c_pins(&i2c3_handler);
	}
	else if(i2cHandle->Instance == I2C4)
	{
	  __HAL_RCC_I2C4_CLK_DISABLE();
	  deinit_i2c_pins(&i2c4_handler);
	}
} 

static void init_i2c_pins(dsy_i2c_handle_t* hi2c) 
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_TypeDef* port;
	for(uint8_t i = 0; i < DSY_I2C_PIN_LAST; i++)
	{
		port = (GPIO_TypeDef*)
			gpio_hal_port_map[hi2c->pin_config[i].port];
		GPIO_InitStruct.Pin
			= gpio_hal_pin_map[hi2c->pin_config[i].pin];
		GPIO_InitStruct.Mode  = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull  = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		switch(hi2c->periph)
		{
			case DSY_I2C_PERIPH_1:
				GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
				break;
			case DSY_I2C_PERIPH_2:
				GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
				break;
			default: break;
		}
		HAL_GPIO_Init(port, &GPIO_InitStruct);
	}
}

static void deinit_i2c_pins(dsy_i2c_handle_t* hi2c) 
{
	GPIO_TypeDef* port;
	uint16_t	  pin;
	for(uint8_t i = 0; i < DSY_I2C_PIN_LAST; i++)
	{
		port = (GPIO_TypeDef*)
			gpio_hal_port_map[hi2c->pin_config[i].port];
		pin = 
			gpio_hal_pin_map[hi2c->pin_config[i].pin];
		HAL_GPIO_DeInit(port, pin);
	}
}

