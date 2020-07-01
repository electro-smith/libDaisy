// WM8371 Codec support.
#include "dev_codec_wm8731.h"
#include "util_hal_map.h"

typedef struct {
  uint8_t mcu_is_master, bitdepth;
  int32_t sample_rate;
  size_t block_size;
  size_t stride;
  I2C_HandleTypeDef *i2c;
}dsy_wm8731_handle_t;

#define W8731_ADDR_0 0x1A
#define W8731_ADDR_1 0x1B
#define W8731_NUM_REGS 10
#define CODEC_ADDRESS           (W8731_ADDR_0 << 1)

#define WAIT_LONG(x) { \
  uint32_t timeout = CODEC_LONG_TIMEOUT; \
  while (x) { if ((timeout--) == 0) return 0; } \
}

#define WAIT(x) { \
  uint32_t timeout = CODEC_TIMEOUT; \
  while (x) { if ((timeout--) == 0) return 0; } \
}

static uint8_t codec_write_control_register(uint8_t address, uint16_t data);
static uint8_t init_codec(uint8_t mcu_is_master, int32_t sample_rate, uint8_t bitdepth);

enum CodecRegister {
  CODEC_REG_LEFT_LINE_IN = 0x00,
  CODEC_REG_RIGHT_LINE_IN = 0x01,
  CODEC_REG_LEFT_HEADPHONES_OUT = 0x02,
  CODEC_REG_RIGHT_HEADPHONES_OUT = 0x03,
  CODEC_REG_ANALOGUE_ROUTING = 0x04,
  CODEC_REG_DIGITAL_ROUTING = 0x05,
  CODEC_REG_POWER_MANAGEMENT = 0x06,
  CODEC_REG_DIGITAL_FORMAT = 0x07,
  CODEC_REG_SAMPLE_RATE = 0x08,
  CODEC_REG_ACTIVE = 0x09,
  CODEC_REG_RESET = 0x0f,
};

enum CodecSettings {
  CODEC_INPUT_0_DB = 0x17,
  CODEC_INPUT_UPDATE_BOTH = 0x40,
  CODEC_HEADPHONES_MUTE = 0x00,
  CODEC_MIC_BOOST = 0x1,
  CODEC_MIC_MUTE = 0x2,
  CODEC_ADC_MIC = 0x4,
  CODEC_ADC_LINE = 0x0,
  CODEC_OUTPUT_DAC_ENABLE = 0x10,
  CODEC_OUTPUT_MONITOR = 0x20,
  CODEC_DEEMPHASIS_NONE = 0x00,
  CODEC_DEEMPHASIS_32K = 0x01,
  CODEC_DEEMPHASIS_44K = 0x02,
  CODEC_DEEMPHASIS_48K = 0x03,
  CODEC_SOFT_MUTE = 0x01,
  CODEC_ADC_HPF = 0x00,
  
  CODEC_POWER_DOWN_LINE_IN = 0x01,
  CODEC_POWER_DOWN_MIC = 0x02,
  CODEC_POWER_DOWN_ADC = 0x04,
  CODEC_POWER_DOWN_DAC = 0x08,
  CODEC_POWER_DOWN_LINE_OUT = 0x10,
  CODEC_POWER_DOWN_OSCILLATOR = 0x20,
  CODEC_POWER_DOWN_CLOCK_OUTPUT = 0x40,
  CODEC_POWER_DOWN_EVERYTHING = 0x80,
  
  CODEC_PROTOCOL_MASK_MSB_FIRST = 0x00,
  CODEC_PROTOCOL_MASK_LSB_FIRST = 0x01,
  CODEC_PROTOCOL_MASK_PHILIPS = 0x02,
  CODEC_PROTOCOL_MASK_DSP = 0x03,
  
  CODEC_FORMAT_MASK_16_BIT = 0x00 << 2,
  CODEC_FORMAT_MASK_20_BIT = 0x01 << 2,
  CODEC_FORMAT_MASK_24_BIT = 0x02 << 2,
  CODEC_FORMAT_MASK_32_BIT = 0x03 << 2,
  
  CODEC_FORMAT_LR_SWAP = 0x20,
  CODEC_FORMAT_MASTER = 0x40,
  CODEC_FORMAT_SLAVE = 0x00,
  CODEC_FORMAT_INVERT_CLOCK = 0x80,
  
  CODEC_RATE_48K_48K = 0x00 << 2,
  CODEC_RATE_8K_8K = 0x03 << 2,
  CODEC_RATE_96K_96K = 0x07 << 2,
  CODEC_RATE_32K_32K = 0x06 << 2,
  CODEC_RATE_44K_44K = 0x08 << 2,
};

enum CodecAnalogSettings
{
	CODEC_ANALOG_MICBOOST	= 0x01,
	CODEC_ANALOG_MUTEMIC	= 0x02,
	CODEC_ANALOG_INSEL		= 0x04,
	CODEC_ANALOG_BYPASS		= 0x08,
	CODEC_ANALOG_DACSEL		= 0x10,
	CODEC_ANALOG_SIDETONE	= 0x20,
	CODEC_ANALOG_SIDEATT_0	= 0x40,
	CODEC_ANALOG_SIDEATT_1	= 0x80,
};
// BEGIN SHENSLEY PORT
dsy_wm8731_handle_t codec_handle;
uint8_t codec_wm8731_init( \
	dsy_i2c_handle *hi2c, \
	uint8_t mcu_is_master, \
	int32_t sample_rate, 
	uint8_t bitdepth) 
{
    codec_handle.sample_rate   = sample_rate;
    codec_handle.mcu_is_master = mcu_is_master;
    codec_handle.i2c           = dsy_hal_map_get_i2c(hi2c);
    codec_handle.bitdepth      = bitdepth;

    uint8_t s;
	s = init_codec(mcu_is_master, sample_rate, bitdepth);
	if (!codec_write_control_register(CODEC_REG_ACTIVE, 0x01)) {
		return 0;
	}
	return s;
}

uint8_t codec_wm8731_enter_bypass(dsy_i2c_handle *hi2c)
{
    uint8_t s;
    uint8_t bypass_mode_byte = 0;
    codec_handle.i2c           = dsy_hal_map_get_i2c(hi2c);
	bypass_mode_byte |= CODEC_ANALOG_BYPASS | CODEC_ANALOG_MUTEMIC;
	s = codec_write_control_register(CODEC_REG_ANALOGUE_ROUTING,
										  bypass_mode_byte);
	s = s && codec_write_control_register(CODEC_REG_ACTIVE, 0x00);
	return s;
}
uint8_t codec_wm8731_exit_bypass(dsy_i2c_handle *hi2c)
{
    uint8_t s;
    uint8_t byte = 0;
	codec_handle.i2c		 = dsy_hal_map_get_i2c(hi2c);
    byte |= CODEC_ANALOG_MUTEMIC | CODEC_ANALOG_DACSEL;
    s = codec_write_control_register(CODEC_REG_ANALOGUE_ROUTING, byte);
    s = s && codec_write_control_register(CODEC_REG_ACTIVE, 0x00);
    return s;
}

static uint8_t init_codec(uint8_t mcu_is_master, int32_t sample_rate, uint8_t bitdepth)
{
  uint8_t s = 1;  // success;
  s = s && codec_write_control_register(CODEC_REG_RESET, 0);
  // Configure L&R inputs
  s = s && codec_write_control_register(CODEC_REG_LEFT_LINE_IN, CODEC_INPUT_0_DB);
  s = s && codec_write_control_register(CODEC_REG_RIGHT_LINE_IN, CODEC_INPUT_0_DB);
  
  // Configure L&R headphone outputs
  s = s && codec_write_control_register(CODEC_REG_LEFT_HEADPHONES_OUT, CODEC_HEADPHONES_MUTE);
  s = s && codec_write_control_register(CODEC_REG_RIGHT_HEADPHONES_OUT, CODEC_HEADPHONES_MUTE);
//  uint8_t hp_config = 0b01110101;
//  s = s && codec_write_control_register(CODEC_REG_LEFT_HEADPHONES_OUT, hp_config);
//  s = s && codec_write_control_register(CODEC_REG_RIGHT_HEADPHONES_OUT, hp_config);

  // Configure analog routing
  s = s && codec_write_control_register(
      CODEC_REG_ANALOGUE_ROUTING,
      CODEC_MIC_MUTE | CODEC_ADC_LINE | CODEC_OUTPUT_DAC_ENABLE);

  // Configure digital routing
  s = s && codec_write_control_register(CODEC_REG_DIGITAL_ROUTING, CODEC_DEEMPHASIS_NONE);

  // Configure power management
  uint8_t power_down_reg = CODEC_POWER_DOWN_MIC | CODEC_POWER_DOWN_CLOCK_OUTPUT;
  if (mcu_is_master) {
    power_down_reg |= CODEC_POWER_DOWN_OSCILLATOR;
  }
    
  s = s && codec_write_control_register(CODEC_REG_POWER_MANAGEMENT, power_down_reg);

  uint8_t format_byte;
  format_byte = CODEC_PROTOCOL_MASK_PHILIPS;
  if(bitdepth == 24)
  {
	  format_byte |= CODEC_FORMAT_MASK_24_BIT;
  }
  else
  {
	  // TODO ADD SUPPORT FOR UP TO 32 bit as well
	  format_byte |= CODEC_FORMAT_MASK_16_BIT;
  }
  format_byte |= mcu_is_master ? CODEC_FORMAT_SLAVE : CODEC_FORMAT_MASTER;

  s = s && codec_write_control_register(CODEC_REG_DIGITAL_FORMAT, format_byte);
  
  uint8_t rate_byte = 0;
  if (mcu_is_master) {
    // According to the WM8731 datasheet, the 32kHz and 96kHz modes require the
    // master clock to be at 12.288 MHz (384 fs / 128 fs). The STM32F4 I2S clock
    // is always at 256 fs. So the 32kHz and 96kHz modes are achieved by
    // pretending that we are doing 48kHz, but with a slower or faster master
    // clock.
    rate_byte = sample_rate == 44100 ? CODEC_RATE_44K_44K : CODEC_RATE_48K_48K;
  } else {
    switch (sample_rate) {
      case 8000:
        rate_byte = CODEC_RATE_8K_8K;
        break;
      case 32000:
        rate_byte = CODEC_RATE_32K_32K;
        break;
      case 44100:
        rate_byte = CODEC_RATE_44K_44K;
        break;
      case 96000:
        rate_byte = CODEC_RATE_96K_96K;
        break;
      case 48000:
      default:
        rate_byte = CODEC_RATE_48K_48K;
        break;
    }
  }
  s = s && codec_write_control_register(CODEC_REG_SAMPLE_RATE, rate_byte);
  // For now codec is not active.
  s = s && codec_write_control_register(CODEC_REG_ACTIVE, 0x00);
  
  return s;

}


uint8_t codec_write_control_register(uint8_t address, uint16_t data)
{
  uint8_t byte_1 = ((address << 1) & 0xfe) | ((data >> 8) & 0x01);
  uint8_t byte_2 = data & 0xff;
  uint8_t buff[2] = { byte_1, byte_2 };
  
  HAL_I2C_Master_Transmit(codec_handle.i2c, CODEC_ADDRESS, buff, 2, 1);
  HAL_Delay(10);

  return 1;  
}

//uint8_t sa_codec_start(dsy_wm8731_handle_t* c, size_t block_size)
//{
//	return 0;
//}

//static int setup_the_codec(dsy_wm8731_handle_t *c, size_t block_size)
//{
//  // Start the codec.
//  if (!codec_write_control_register(CODEC_REG_ACTIVE, 0x01)) {
//    return 0;
//  }
//  
//  size_t stride = 1;
//  if (!c->mcu_is_master) {
//    // When the WM8731 is the master, the data is sent with padding.
//    switch (c->sample_rate) {
//      case 32000:
//        stride = 3;
//        break;
//      case 48000:
//        stride = 2;
//        break;
//      case 96000:
//        stride = 4;
//        break;
//    }
//  }
//
//  c->block_size = block_size;
//  stride = 1;
//  c->stride = stride;
//	//c->stride = 3;
//
//  size_t buffer_size = 2 * stride * block_size * 2;
//  
//  HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)c->rx_dma_buffer, buffer_size);
//  HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)c->tx_dma_buffer, buffer_size);
//  
//  return 1;
//}

//uint8_t sa_codec_start_callback(dsy_wm8731_handle_t* c, size_t block_size, sa_codec_fill_buffer_callback callback)
//{
//    //c->callback = callback;
//    return setup_the_codec(c, block_size);
//}
//
//int sa_audio_start_callback(
//        dsy_wm8731_handle_t* c,
//        int block_size, 
//        sa_audio_callback process)
//{
//    c->process = process;
//    return setup_the_codec(c, block_size);
//}
//
//void sa_codec_stop(dsy_wm8731_handle_t* c)
//{
//  //DMA_Cmd(AUDIO_I2S_DMA_STREAM, DISABLE);
//  //DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
//}

//void sa_audio_fill(dsy_wm8731_handle_t* c, size_t offset)
//{
//  //c->stride = 8;
//  if (c->process) {
//    offset *= c->block_size * c->stride * 2;
//	  //offset = 0;
//
//	  short* in = &c->rx_dma_buffer[offset];
//	  short* out = &c->tx_dma_buffer[offset];
//    /*
//    if (c->stride) {
//      // Undo the padding from the WM8731.
//      for (size_t i = 1; i < c->block_size * 1; ++i) {
//        in[i] = in[i];
//      }
//    }
//    */
//    (*c->process)((codec_frame_t*)in, (codec_frame_t*)out, (size_t)c->block_size);
//    /*
//    for (size_t i = 0; i < c->block_size * 4; i++) {
//      out[i] = 32767;
//    }
//    */
//    /*
//    if (c->stride) {
//      // Pad for the WM8731.
//      for (size_t i = c->block_size * 2 - 1; i > 0; --i) {
//        out[i * c->stride] = out[i];
//      }
//    }
//    */
//  }
//}
//
//uint8_t sa_codec_set_line_input_gain(dsy_wm8731_handle_t* c, int32_t channel, int32_t gain)
//{
//  return codec_write_control_register(CODEC_REG_LEFT_LINE_IN + channel, gain);
//}
//uint8_t sa_codec_set_stereo_line_input_gain(dsy_wm8731_handle_t* c, int32_t gain)
//{
//  return codec_write_control_register(0, gain) && codec_write_control_register(1, gain);
//}
//
//
//uint8_t sa_codec_initialize_audio_interface(uint8_t mcu_is_master, int32_t sample_rate)
//{
//  
//  //MX_SAI1_Init();
//  
//  return 1;
//}


