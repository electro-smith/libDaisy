#include "dev_codec_pcm3060.h"
#include "util_hal_map.h"
//#define CODEC_DEVICE_ADDR (0b10001100)
#define CODEC_DEVICE_ADDR 0x8c
#define REGISTER_MODE1 0x40
#define DAC_POWERSAVE_CTRL (4)
#define ADC_POWERSAVE_CTRL (5)
#define MODE_CONTROL_RESET (7)
#define SYSTEM_RESET (6)

#define REGISTER_ADC_CTL 0x48
#define REGISTER_DAC_CTL 0x43
#define FMT_I2S ((0 << 0) | (0 << 1))
#define FMT_MSB ((0 << 1) | (1 << 0))
#define FMT_LSB ((1 << 1) | (0 << 0))
#define FMT_16BIT ((1 << 1) | (1 << 0))

static HAL_StatusTypeDef
send_cmd(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t byte);

void codec_pcm3060_init(dsy_i2c_handle *hi2c)
{
    // Crude as heck
    // Ideal Register 64
    // 7	6		5		4		3	2	1	0
    // MRST	SRST	ADPSV	DAPSV	RSV	RSV	RSV S/E
    // 1	1		0		0		0	0	0	0
    HAL_StatusTypeDef ret;
    //uint8_t reset_byte = 0b00000000;
    uint8_t reset_byte = 0;
    uint8_t run_byte   = 0b11000000;
    uint8_t addr       = CODEC_DEVICE_ADDR;
    uint8_t buff[2];

    buff[0] = REGISTER_MODE1;
    // Reset the mode control register.
    // Should set all things to default.
    buff[1] = reset_byte | (1 << SYSTEM_RESET) | (1 << DAC_POWERSAVE_CTRL)
              | (1 << ADC_POWERSAVE_CTRL); // Should be 0b01110000 (MRST)
    I2C_HandleTypeDef *hal_hi2c;
    hal_hi2c = dsy_hal_map_get_i2c(hi2c);
    ret = HAL_I2C_Master_Transmit(hal_hi2c, addr, buff, 2, 1);
    if(ret != HAL_OK)
    {
        // Do something
    }
    // System Reset
    // Should resync signals.
    buff[0] = REGISTER_MODE1;
    buff[1] = reset_byte | (1 << MODE_CONTROL_RESET) | (1 << DAC_POWERSAVE_CTRL)
              | (1 << ADC_POWERSAVE_CTRL);
    ret = HAL_I2C_Master_Transmit(hal_hi2c, addr, buff, 2, 1);
    if(ret != HAL_OK)
    {
        // Do something
    }
    HAL_Delay(100);
    // Set ADC to 16-bit
    //	buff[0] = REGISTER_ADC_CTL;
    //	buff[1] = 0;
    //	buff[1] |= (FMT_16);
    //	ret = HAL_I2C_Master_Transmit(hi2c, addr, buff, 2, 1);
    //	if (ret != HAL_OK)
    //	{
    //		// Do something
    //	}
    uint8_t cmd;
    cmd = 0 | (FMT_I2S);
    send_cmd(hal_hi2c, REGISTER_ADC_CTL, cmd);
    send_cmd(hal_hi2c, REGISTER_DAC_CTL, cmd);
    // Shut down ADC / DAC Power save.
    buff[0] = REGISTER_MODE1;
    buff[1] = run_byte;
    ret     = HAL_I2C_Master_Transmit(hal_hi2c, addr, buff, 2, 1);
    if(ret != HAL_OK)
    {
        // Do something
    }
}

static HAL_StatusTypeDef
send_cmd(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t byte)
{
    uint8_t buff[2];
    uint8_t addr = CODEC_DEVICE_ADDR;
    buff[0]      = reg;
    buff[1]      = byte;
    return HAL_I2C_Master_Transmit(hi2c, addr, buff, 2, 1);
}
