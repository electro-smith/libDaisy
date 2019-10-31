#include "libdaisy.h"
#include "dsy_leddriver.h"
#include "dsy_i2c.h"

#define PCA9685_I2C_BASE_ADDRESS 0b10000000
#define PCA9685_MODE1 0x00  // location for Mode1 register address
#define PCA9685_MODE2 0x01  // location for Mode2 reigster address
#define PCA9685_LED0 0x06   // location for start of LED0 registers
#define PRE_SCALE_MODE 0xFE //location for setting prescale (clock speed)

#define CHANNELS_PER_DRIVER 16

typedef struct
{
	color_t color;
	//uint16_t *r, *g, *b;
	uint16_t addr_r, addr_g, addr_b; // 0-15
	uint16_t drv_r, drv_g, drv_b;	// 0-3
} rgb_led_t;

typedef struct
{
	uint16_t bright;
	uint16_t addr, drv;
} led_t;

typedef enum
{
	DRIVER_1,
	//	DRIVER_2,
	//	DRIVER_3,
	//	DRIVER_4,
	DRIVER_LAST
} leddriver_names;

// channels * bytes_to_send + start_tx_byte
#define LED_BUFF_SIZE ((16 * 4) + 1)
typedef struct
{
	led_t	 leds[CHANNELS_PER_DRIVER];
	uint16_t *sorted_bright[DRIVER_LAST][16];
	uint16_t  dummy_bright; // not sure if NULL will break things later.
	float master_dim;
	uint8_t			   temp_buff[LED_BUFF_SIZE];
	uint8_t			   current_drv;
	color_t			   standard_colors[LED_COLOR_LAST];
	I2C_HandleTypeDef *i2c;
	dsy_i2c_handle_t * dsy_i2c;
} dsy_led_driver_t;

dsy_led_driver_t leddriver;

static void init_rgb_leds();
static void init_single_led(uint8_t name, uint8_t addr, uint8_t drv);
static void gen_sorted_table();


void dsy_led_driver_init(dsy_i2c_handle_t *dsy_i2c)
{
	uint8_t address = PCA9685_I2C_BASE_ADDRESS;
	uint8_t init_buff[2];
	leddriver.dummy_bright = 0;
	leddriver.master_dim   = 1.0f;
	leddriver.dsy_i2c	  = dsy_i2c;
	leddriver.i2c		   = dsy_i2c_hal_handle(dsy_i2c);
	dsy_i2c_init(dsy_i2c);
	init_rgb_leds();
	gen_sorted_table();
	for(uint8_t i = 0; i < CHANNELS_PER_DRIVER; i++)
	{
		//		leddriver.leds[i].color.red = 4095;
		//		leddriver.leds[i].color.green = 0;
		//		leddriver.leds[i].color.blue = 0;
		leddriver.leds[i].bright = 4095;
	}
	//HAL_GPIO_WritePin(LED_OE_GPIO_Port, LED_OE_Pin, 0);
	for(uint8_t i = 0; i < DRIVER_LAST; i++)
	{
		for(uint8_t j = 0; j < LED_BUFF_SIZE; j++)
		{
			leddriver.temp_buff[i] = 0;
		}
		init_buff[0] = PCA9685_MODE1;
		init_buff[1] = 0x00;
		HAL_I2C_Master_Transmit(
			leddriver.i2c, address | (i << 1), init_buff, 2, 1);
		HAL_Delay(20);
		init_buff[0] = PCA9685_MODE1;
		init_buff[1] = 0x00;
		HAL_I2C_Master_Transmit(
			leddriver.i2c, address | (i << 1), init_buff, 2, 1);
		HAL_Delay(20);
		init_buff[0] = PCA9685_MODE1;
		init_buff[1] = 0b00100000;
		HAL_I2C_Master_Transmit(
			leddriver.i2c, address | (i << 1), init_buff, 2, 1);
		init_buff[0] = PCA9685_MODE2;
		//init_buff[1] = 0b00011101;
		//init_buff[1] = 0b00010001;
		init_buff[1] = 0b00010000;
		HAL_I2C_Master_Transmit(
			leddriver.i2c, address | (i << 1), init_buff, 2, 1);
	}
	leddriver.current_drv = 0;
	//uint8_t *buff = leddriver.temp_buff[0];

	// Init standard colors
	leddriver.standard_colors[LED_COLOR_RED].red	  = 4095;
	leddriver.standard_colors[LED_COLOR_RED].green	= 0;
	leddriver.standard_colors[LED_COLOR_RED].blue	 = 0;
	leddriver.standard_colors[LED_COLOR_GREEN].red	= 0;
	leddriver.standard_colors[LED_COLOR_GREEN].green  = 4095;
	leddriver.standard_colors[LED_COLOR_GREEN].blue   = 0;
	leddriver.standard_colors[LED_COLOR_BLUE].red	 = 0;
	leddriver.standard_colors[LED_COLOR_BLUE].green   = 0;
	leddriver.standard_colors[LED_COLOR_BLUE].blue	= 4095;
	leddriver.standard_colors[LED_COLOR_WHITE].red	= 4095;
	leddriver.standard_colors[LED_COLOR_WHITE].green  = 4095;
	leddriver.standard_colors[LED_COLOR_WHITE].blue   = 4095;
	leddriver.standard_colors[LED_COLOR_PURPLE].red   = 1024;
	leddriver.standard_colors[LED_COLOR_PURPLE].green = 0;
	leddriver.standard_colors[LED_COLOR_PURPLE].blue  = 4095;
	leddriver.standard_colors[LED_COLOR_CYAN].red	 = 0;
	leddriver.standard_colors[LED_COLOR_CYAN].green   = 1024;
	leddriver.standard_colors[LED_COLOR_CYAN].blue	= 3095;
	leddriver.standard_colors[LED_COLOR_GOLD].red	 = 4095;
	leddriver.standard_colors[LED_COLOR_GOLD].green   = 1600;
	leddriver.standard_colors[LED_COLOR_GOLD].blue	= 0;
	leddriver.standard_colors[LED_COLOR_OFF].red	  = 0;
	leddriver.standard_colors[LED_COLOR_OFF].green	= 0;
	leddriver.standard_colors[LED_COLOR_OFF].blue	 = 0;
	dsy_led_driver_update();
}
void dsy_led_driver_update()
{
	uint8_t  on;
	uint16_t off;
	uint8_t  drvr		 = leddriver.current_drv;
	uint8_t *output_buff = leddriver.temp_buff;
	uint8_t  idx		 = 0;
	on					 = 0;
	output_buff[0]		 = PCA9685_LED0;
	idx					 = 1;
	for(uint8_t i = 0; i < 16; i++)
	{
		if(drvr < DRIVER_LAST)
		{
			off = *(leddriver.sorted_bright[drvr][i]);
		}
		else
		{
			off = 0;
		}
		output_buff[idx]	 = on;
		output_buff[idx + 1] = on;
		output_buff[idx + 2] = (uint8_t)(off & 0xff);
		output_buff[idx + 3] = (uint8_t)((off >> 8) & 0xff);
		idx += 4;
	}
	uint8_t driveraddr = PCA9685_I2C_BASE_ADDRESS | (drvr << 1);
	HAL_I2C_Master_Transmit(
		leddriver.i2c, driveraddr, output_buff, LED_BUFF_SIZE, 5);
	leddriver.current_drv += 1;
	if(leddriver.current_drv > DRIVER_LAST - 1)
	{
		leddriver.current_drv = 0;
	}
}
void dsy_led_driver_set_led(uint8_t idx, float bright)
{
	float temp;
	temp = 4095.0f * bright;
	if(temp < 0.0f)
	{
		temp = 0.0f;
	}
	leddriver.leds[idx].bright = (uint16_t)temp;
}

//#define TIME_LEDS 1
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
#ifdef TIME_LEDS
	HAL_GPIO_WritePin(CLK_OUT_GPIO_Port, CLK_OUT_Pin, GPIO_PIN_RESET);
#endif
//dsy_led_driver_update();
//HAL_I2C_Master_Transmit_DMA(hi2c, PCA9685_I2C_BASE_ADDRESS | (leddriver.current_drv << 1), leddriver.temp_buff[leddriver.current_drv], LED_BUFF_SIZE);
#ifdef TIME_LEDS
	HAL_GPIO_WritePin(CLK_OUT_GPIO_Port, CLK_OUT_Pin, GPIO_PIN_SET);
#endif
}

color_t *dsy_led_driver_color_by_name(uint8_t name)
{
	color_t *c;
	if(name < LED_COLOR_LAST)
	{
		c = &leddriver.standard_colors[name];
	}
	else
	{
		c = &leddriver.standard_colors[LED_COLOR_OFF];
	}
	return c;
}

static void init_rgb_leds()
{
	for(uint8_t i = 0; i < CHANNELS_PER_DRIVER; i++)
	{
		init_single_led(i, i, 0);
	}
}
static void gen_sorted_table()
{
	uint8_t tdrv, taddr;
	for(uint8_t i = 0; i < CHANNELS_PER_DRIVER; i++)
	{
		tdrv								 = leddriver.leds[i].drv;
		taddr								 = leddriver.leds[i].addr;
		leddriver.sorted_bright[tdrv][taddr] = &leddriver.leds[i].bright;
	}
}

static void init_single_led(uint8_t name, uint8_t addr, uint8_t drv)
{
	leddriver.leds[name].addr = addr;
	leddriver.leds[name].drv  = drv;
}
