#include "libdaisy.h"
#include "dsy_leddriver.h"
#include "i2c.h"

#define PCA9685_I2C_BASE_ADDRESS 0b10000000
#define PCA9685_MODE1 0x00  // location for Mode1 register address
#define PCA9685_MODE2 0x01  // location for Mode2 reigster address
#define PCA9685_LED0 0x06   // location for start of LED0 registers
#define PRE_SCALE_MODE 0xFE //location for setting prescale (clock speed)

typedef struct
{
	color_t color;
	//uint16_t *r, *g, *b;
	uint16_t addr_r, addr_g, addr_b; // 0-15
	uint16_t drv_r, drv_g, drv_b; // 0-3
}rgb_led_t;

typedef struct
{
	uint16_t bright;
	uint16_t addr, drv;
}led_t;

typedef enum {
	DRIVER_1,
//	DRIVER_2,
//	DRIVER_3,
//	DRIVER_4,
	DRIVER_LAST
} leddriver_names;

// channels * bytes_to_send + start_tx_byte
#define LED_BUFF_SIZE ((16 * 4) + 1)
typedef struct {
	//rgb_led_t leds[LED_LAST];
	led_t leds[LED_LAST];
	uint16_t *sorted_bright[DRIVER_LAST][16];
	uint16_t dummy_bright; // not sure if NULL will break things later.
	//color_t leds[LED_LAST];
	float master_dim;
	//uint8_t i2c_buff[3][LED_BUFF_SIZE];
	uint8_t temp_buff[LED_BUFF_SIZE];
	uint8_t current_drv;
	color_t standard_colors[LED_COLOR_LAST];
	I2C_HandleTypeDef *i2c;
} dsy_led_driver_t;

dsy_led_driver_t leddriver;

static void init_rgb_leds();
static void init_single_rgb_led(uint8_t name,
								uint8_t addr_r,
								uint8_t addr_g,
								uint8_t addr_b,
								uint8_t drv_r,
								uint8_t drv_g,
								uint8_t drv_b);

static void init_single_led(uint8_t name, uint8_t addr, uint8_t drv);
static void gen_sorted_table();


void dsy_led_driver_init(uint8_t board)
{
	uint8_t address = PCA9685_I2C_BASE_ADDRESS;
	uint8_t init_buff[2];
	leddriver.dummy_bright = 0;
	leddriver.master_dim   = 1.0f;
	switch(board)
	{
		case DSY_SYS_BOARD_DAISY_SEED:
			dsy_i2c1_init(board);
			leddriver.i2c = &hi2c1;
			break;
		case DSY_SYS_BOARD_EURORACK_TESTER:
			dsy_i2c2_init(board);
			leddriver.i2c = &hi2c2;
			break;
		default:
			dsy_i2c1_init(board);
			leddriver.i2c = &hi2c1;
			break;
	}
	init_rgb_leds();
	gen_sorted_table();
	for(uint8_t i = 0; i < LED_LAST; i++)
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
	uint16_t *temp;
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
	//	temp = (float)c->red * bright;
	//	leddriver.leds[idx].color.red  = (uint16_t)temp;
	//	temp = (float)c->green * bright;
	//	leddriver.leds[idx].color.green  = (uint16_t)temp;
	//	temp = (float)c->blue * bright;
	//	leddriver.leds[idx].color.blue = (uint16_t)temp;
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
	// Set up addresses, etc. for each LED.
	// LEDDriver 1
	//	init_single_led(LED_TRUNK_1, 0, 1, 2, 0, 0, 0);
	//	init_single_led(LED_TRUNK_2, 3, 4, 5, 0, 0, 0);
	//	init_single_led(LED_TRUNK_3, 6, 7, 8, 0, 0, 0);
	//	init_single_led(LED_CHANNEL, 9, 10, 11, 0, 0, 0);
	//	init_single_rgb_led(LED_RESET, 12, 13, 14, 0, 0, 0);
	// LEDDriver 2
	//	init_single_led(LED_CENTER_2, 0, 1, 2, 1, 1, 1);
	//	init_single_led(LED_TRUNK_5, 3, 4, 5, 1, 1, 1);
	//	init_single_led(LED_TRUNK_6, 6, 7, 8, 1, 1, 1);
	//	init_single_led(LED_TRUNK_4, 9, 10, 11, 1, 1, 1);
	//	init_single_led(LED_CENTER_1, 12, 13, 14, 1, 1, 1);
	// LEDDriver 3
	//	init_single_led(LED_SHIFT, 0, 1, 2, 2, 2, 2);
	//	init_single_led(LED_TRUNK_8, 3, 4, 5, 2, 2, 2);
	//	init_single_led(LED_TRUNK_7, 6, 7, 8, 2, 2, 2);
	//	init_single_led(LED_CENTER_4, 9, 10, 11, 2, 2, 2);
	//	init_single_led(LED_CENTER_3, 12, 13, 14, 2, 2, 2);
	// LEDDriver 4
	//	init_single_led(LED_GATE_1, 0, 1, 2, 3, 3, 3);
	//	init_single_led(LED_GATE_2, 3, 4, 5, 3, 3, 3);
	//	init_single_led(LED_CLOCK, 6, 7, 8, 3, 3, 3);
	//	init_single_led(LED_OUT_1, 9, 10, 11, 3, 3, 3);
	//	init_single_led(LED_OUT_2, 12, 13, 14, 3, 3, 3);
	for(uint8_t i = 0; i < LED_LAST; i++)
	{
		init_single_led(i, i, 0);
	}
}
static void gen_sorted_table()
{
	uint8_t temp[DRIVER_LAST][16];
	uint8_t dummy_addr
		= 16; // This works because all of the pin 16s are floating
	uint8_t tdrv, taddr;
	for(uint8_t i = 0; i < LED_LAST; i++)
	{
		//		tdrv = leddriver.leds[i].drv_r;
		//		taddr = leddriver.leds[i].addr_r;
		//		leddriver.sorted_bright[tdrv][taddr] = &leddriver.leds[i].color.red;
		//		tdrv = leddriver.leds[i].drv_g;
		//		taddr = leddriver.leds[i].addr_g;
		//		leddriver.sorted_bright[tdrv][taddr] = &leddriver.leds[i].color.green;
		//		tdrv = leddriver.leds[i].drv_b;
		//		taddr = leddriver.leds[i].addr_b;
		//		leddriver.sorted_bright[tdrv][taddr] = &leddriver.leds[i].color.blue;
		tdrv								 = leddriver.leds[i].drv;
		taddr								 = leddriver.leds[i].addr;
		leddriver.sorted_bright[tdrv][taddr] = &leddriver.leds[i].bright;
	}
	// Handle floating pins
	//	for (uint8_t i = 0; i < DRIVER_LAST; i++)
	//	{
	//		leddriver.sorted_bright[i][15] = &leddriver.dummy_bright;
	//	}
}

static void init_single_rgb_led(uint8_t name,
								uint8_t addr_r,
								uint8_t addr_g,
								uint8_t addr_b,
								uint8_t drv_r,
								uint8_t drv_g,
								uint8_t drv_b)
{
	//	leddriver.leds[name].addr_r = addr_r;
	//	leddriver.leds[name].addr_g = addr_g;
	//	leddriver.leds[name].addr_b = addr_b;
	//	leddriver.leds[name].drv_r = drv_r;
	//	leddriver.leds[name].drv_g = drv_g;
	//	leddriver.leds[name].drv_b = drv_b;
}

static void init_single_led(uint8_t name, uint8_t addr, uint8_t drv)
{
	leddriver.leds[name].addr = addr;
	leddriver.leds[name].drv  = drv;
}
