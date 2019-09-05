// WM8371 Codec support.
#include "codec_wm8731.h"

#define CODEC_I2C                      I2C2
#define CODEC_I2C_CLK                  RCC_APB1Periph_I2C2
#define CODEC_I2C_GPIO_CLOCK           RCC_AHB1Periph_GPIOB
#define CODEC_I2C_GPIO_AF              GPIO_AF_I2C2
#define CODEC_I2C_GPIO                 GPIOB
#define CODEC_I2C_SCL_PIN              GPIO_Pin_10
#define CODEC_I2C_SDA_PIN              GPIO_Pin_11
#define CODEC_I2C_SCL_PINSRC           GPIO_PinSource10
#define CODEC_I2C_SDA_PINSRC           GPIO_PinSource11
#define CODEC_TIMEOUT                  ((uint32_t)0x1000)
#define CODEC_LONG_TIMEOUT             ((uint32_t)(300 * CODEC_TIMEOUT))
#define CODEC_I2C_SPEED                100000

#define CODEC_I2S                      SPI2
#define CODEC_I2S_EXT                  I2S2ext
#define CODEC_I2S_CLK                  RCC_APB1Periph_SPI2
#define CODEC_I2S_ADDRESS              0x4000380C
#define CODEC_I2S_EXT_ADDRESS          0x4000340C
#define CODEC_I2S_GPIO_AF              GPIO_AF_SPI2
#define CODEC_I2S_IRQ                  SPI2_IRQn
#define CODEC_I2S_EXT_IRQ              SPI2_IRQn
#define CODEC_I2S_GPIO_CLOCK           (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOB)
#define CODEC_I2S_WS_PIN               GPIO_Pin_12
#define CODEC_I2S_SCK_PIN              GPIO_Pin_13
#define CODEC_I2S_SDI_PIN              GPIO_Pin_14
#define CODEC_I2S_SDO_PIN              GPIO_Pin_15
#define CODEC_I2S_MCK_PIN              GPIO_Pin_6
#define CODEC_I2S_WS_PINSRC            GPIO_PinSource12
#define CODEC_I2S_SCK_PINSRC           GPIO_PinSource13
#define CODEC_I2S_SDI_PINSRC           GPIO_PinSource14
#define CODEC_I2S_SDO_PINSRC           GPIO_PinSource15
#define CODEC_I2S_MCK_PINSRC           GPIO_PinSource6
#define CODEC_I2S_GPIO                 GPIOB
#define CODEC_I2S_MCK_GPIO             GPIOC
#define AUDIO_I2S_IRQHandler           SPI2_IRQHandler

#define AUDIO_DMA_PERIPH_DATA_SIZE     DMA_PeripheralDataSize_HalfWord
#define AUDIO_DMA_MEM_DATA_SIZE        DMA_MemoryDataSize_HalfWord
#define AUDIO_I2S_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_I2S_DMA_STREAM           DMA1_Stream4
#define AUDIO_I2S_DMA_DREG             CODEC_I2S_ADDRESS
#define AUDIO_I2S_DMA_CHANNEL          DMA_Channel_0
#define AUDIO_I2S_DMA_IRQ              DMA1_Stream4_IRQn
#define AUDIO_I2S_DMA_FLAG_TC          DMA_FLAG_TCIF4
#define AUDIO_I2S_DMA_FLAG_HT          DMA_FLAG_HTIF4
#define AUDIO_I2S_DMA_FLAG_FE          DMA_FLAG_FEIF4
#define AUDIO_I2S_DMA_FLAG_TE          DMA_FLAG_TEIF4
#define AUDIO_I2S_DMA_FLAG_DME         DMA_FLAG_DMEIF4
#define AUDIO_I2S_EXT_DMA_STREAM       DMA1_Stream3
#define AUDIO_I2S_EXT_DMA_DREG         CODEC_I2S_EXT_ADDRESS
#define AUDIO_I2S_EXT_DMA_CHANNEL      DMA_Channel_3
#define AUDIO_I2S_EXT_DMA_IRQ          DMA1_Stream3_IRQn
#define AUDIO_I2S_EXT_DMA_FLAG_TC      DMA_FLAG_TCIF3
#define AUDIO_I2S_EXT_DMA_FLAG_HT      DMA_FLAG_HTIF3
#define AUDIO_I2S_EXT_DMA_FLAG_FE      DMA_FLAG_FEIF3
#define AUDIO_I2S_EXT_DMA_FLAG_TE      DMA_FLAG_TEIF3
#define AUDIO_I2S_EXT_DMA_FLAG_DME     DMA_FLAG_DMEIF3
#define AUDIO_I2S_EXT_DMA_REG          DMA1
#define AUDIO_I2S_EXT_DMA_ISR          LISR
#define AUDIO_I2S_EXT_DMA_IFCR         LIFCR

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
// BEGIN SHENSLEY PORT
sa_codec codec_handle;
uint8_t sa_codec_init( \
	I2C_HandleTypeDef *hi2c, \
	uint8_t mcu_is_master, \
	int32_t sample_rate) 
{
  //c->callback = NULL;
  codec_handle.sample_rate = sample_rate;
  codec_handle.mcu_is_master = mcu_is_master;
	codec_handle.i2c = hi2c;

/*
  return sa_codec_initialize_gpio() && \
      sa_codec_initialize_control_interface() && \
      sa_codec_initialize_audio_interface(mcu_is_master, sample_rate) && \
      sa_codec_initialize_codec(mcu_is_master, sample_rate) && \
      sa_codec_initialize_dma(c);
*/
/* test init - found that past the 24th sample is not getting updated.
  int tempbs = 24;
  int tempstride = 2;
  int tempframes = 2;
  size_t temp = tempbs * tempstride * tempframes / 4;
  for (size_t i = 0; i < SA_CODEC_BLOCK_SIZE_MAX * 8 * 2; i++) {
    if (i > temp) {
		  c->tx_dma_buffer[i] = 32767;
		  c->rx_dma_buffer[i] = 32767;
    } else {
		  c->tx_dma_buffer[i] = 0;
		  c->rx_dma_buffer[i] = 0;
    }
  }
*/
	uint8_t s;
	s = sa_codec_initialize_codec(mcu_is_master, sample_rate);
	s |= sa_codec_initialize_audio_interface(mcu_is_master, sample_rate);
	  if (!sa_codec_write_control_register(CODEC_REG_ACTIVE, 0x01)) {
		return 0;
	  }
	return s;
//	return sa_codec_initialize_codec(mcu_is_master, sample_rate) && \
//    sa_codec_initialize_audio_interface(mcu_is_master, sample_rate);
}

uint8_t sa_codec_start(sa_codec* c, size_t block_size)
{
	return 0;
}

static int setup_the_codec(sa_codec *c, size_t block_size)
{
  // Start the codec.
  if (!sa_codec_write_control_register(CODEC_REG_ACTIVE, 0x01)) {
    return 0;
  }
  /*
  if (block_size > SA_CODEC_BLOCK_SIZE_MAX) {
    return 0;
  }
  */
  
/*
  if (!c->mcu_is_master) {
    while(GPIO_ReadInputDataBit(CODEC_I2S_GPIO, CODEC_I2S_WS_PIN));
    while(!GPIO_ReadInputDataBit(CODEC_I2S_GPIO, CODEC_I2S_WS_PIN));
  }
  
  // Enable the I2S TX and RX peripherals.
  if ((CODEC_I2S->I2SCFGR & 0x0400) == 0){
    I2S_Cmd(CODEC_I2S, ENABLE);
  }
  if ((CODEC_I2S_EXT->I2SCFGR & 0x0400) == 0){
    I2S_Cmd(CODEC_I2S_EXT, ENABLE);
  }
*/
  
  //c->dma_init_tx.DMA_Memory0BaseAddr = (uint32_t)(c->tx_dma_buffer);
  //c->dma_init_rx.DMA_Memory0BaseAddr = (uint32_t)(c->rx_dma_buffer);

  size_t stride = 1;
  if (!c->mcu_is_master) {
    // When the WM8731 is the master, the data is sent with padding.
    switch (c->sample_rate) {
      case 32000:
        stride = 3;
        break;
      case 48000:
        stride = 2;
        break;
      case 96000:
        stride = 4;
        break;
    }
  }

  c->block_size = block_size;
  stride = 1;
  c->stride = stride;
	//c->stride = 3;

  size_t buffer_size = 2 * stride * block_size * 2;
  
  HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)c->rx_dma_buffer, buffer_size);
  HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)c->tx_dma_buffer, buffer_size);
  
  return 1;
}

uint8_t sa_codec_start_callback(sa_codec* c, size_t block_size, sa_codec_fill_buffer_callback callback)
{
    //c->callback = callback;
    return setup_the_codec(c, block_size);
}

int sa_audio_start_callback(
        sa_codec* c,
        int block_size, 
        sa_audio_callback process)
{
    c->process = process;
    return setup_the_codec(c, block_size);
}

void sa_codec_stop(sa_codec* c)
{
  //DMA_Cmd(AUDIO_I2S_DMA_STREAM, DISABLE);
  //DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
}

void sa_audio_fill(sa_codec* c, size_t offset)
{
  //c->stride = 8;
  if (c->process) {
    offset *= c->block_size * c->stride * 2;
	  //offset = 0;

	  short* in = &c->rx_dma_buffer[offset];
	  short* out = &c->tx_dma_buffer[offset];
    /*
    if (c->stride) {
      // Undo the padding from the WM8731.
      for (size_t i = 1; i < c->block_size * 1; ++i) {
        in[i] = in[i];
      }
    }
    */
    (*c->process)((codec_frame_t*)in, (codec_frame_t*)out, (size_t)c->block_size);
    /*
    for (size_t i = 0; i < c->block_size * 4; i++) {
      out[i] = 32767;
    }
    */
    /*
    if (c->stride) {
      // Pad for the WM8731.
      for (size_t i = c->block_size * 2 - 1; i > 0; --i) {
        out[i * c->stride] = out[i];
      }
    }
    */
  }
}

uint8_t sa_codec_set_line_input_gain(sa_codec* c, int32_t channel, int32_t gain)
{
  return sa_codec_write_control_register(CODEC_REG_LEFT_LINE_IN + channel, gain);
}
uint8_t sa_codec_set_stereo_line_input_gain(sa_codec* c, int32_t gain)
{
  return sa_codec_write_control_register(0, gain) && sa_codec_write_control_register(1, gain);
}


uint8_t sa_codec_initialize_audio_interface(uint8_t mcu_is_master, int32_t sample_rate)
{
  
  //MX_SAI1_Init();
  
  return 1;
}

uint8_t sa_codec_initialize_codec(uint8_t mcu_is_master, int32_t sample_rate)
{
  uint8_t s = 1;  // success;
  s = s && sa_codec_write_control_register(CODEC_REG_RESET, 0);
  // Configure L&R inputs
  s = s && sa_codec_write_control_register(CODEC_REG_LEFT_LINE_IN, CODEC_INPUT_0_DB);
  s = s && sa_codec_write_control_register(CODEC_REG_RIGHT_LINE_IN, CODEC_INPUT_0_DB);
  
  // Configure L&R headphone outputs
  //s = s && sa_codec_write_control_register(CODEC_REG_LEFT_HEADPHONES_OUT, CODEC_HEADPHONES_MUTE);
  //s = s && sa_codec_write_control_register(CODEC_REG_RIGHT_HEADPHONES_OUT, CODEC_HEADPHONES_MUTE);
  uint8_t hp_config = 0b01110101;
  s = s && sa_codec_write_control_register(CODEC_REG_LEFT_HEADPHONES_OUT, hp_config);
  s = s && sa_codec_write_control_register(CODEC_REG_RIGHT_HEADPHONES_OUT, hp_config);

  // Configure analog routing
  s = s && sa_codec_write_control_register(
      CODEC_REG_ANALOGUE_ROUTING,
      CODEC_MIC_MUTE | CODEC_ADC_LINE | CODEC_OUTPUT_DAC_ENABLE);

  // Configure digital routing
  s = s && sa_codec_write_control_register(CODEC_REG_DIGITAL_ROUTING, CODEC_DEEMPHASIS_NONE);

  // Configure power management
  uint8_t power_down_reg = CODEC_POWER_DOWN_MIC | CODEC_POWER_DOWN_CLOCK_OUTPUT;
  if (mcu_is_master) {
    power_down_reg |= CODEC_POWER_DOWN_OSCILLATOR;
  }
    
  s = s && sa_codec_write_control_register(CODEC_REG_POWER_MANAGEMENT, power_down_reg);
  
  uint8_t format_byte = CODEC_PROTOCOL_MASK_PHILIPS | CODEC_FORMAT_MASK_16_BIT;
  format_byte |= mcu_is_master ? CODEC_FORMAT_SLAVE : CODEC_FORMAT_MASTER;

  s = s && sa_codec_write_control_register(CODEC_REG_DIGITAL_FORMAT, format_byte);
  
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
  s = s && sa_codec_write_control_register(CODEC_REG_SAMPLE_RATE, rate_byte);

  // For now codec is not active.
  s = s && sa_codec_write_control_register(CODEC_REG_ACTIVE, 0x00);
  
  return s;

}


uint8_t sa_codec_write_control_register(uint8_t address, uint16_t data)
{
  uint8_t byte_1 = ((address << 1) & 0xfe) | ((data >> 8) & 0x01);
  uint8_t byte_2 = data & 0xff;
  uint8_t buff[2] = { byte_1, byte_2 };
  
  //HAL_I2C_Master_Transmit(&hi2c2, CODEC_ADDRESS, buff, 2, 1);
  HAL_I2C_Master_Transmit(codec_handle.i2c, CODEC_ADDRESS, buff, 2, 1);
  HAL_Delay(10);

  return 1;  
}

