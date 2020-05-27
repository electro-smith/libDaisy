#include <stm32h7xx_hal.h>
#include "per_adc.h"
#include "util_hal_map.h"

using namespace daisy;

#define PIN_CHN_3 {DSY_GPIOA, 6}
#define PIN_CHN_4 {DSY_GPIOC, 4}
#define PIN_CHN_5 {DSY_GPIOB, 1}
#define PIN_CHN_7 {DSY_GPIOA, 7}
#define PIN_CHN_8 {DSY_GPIOC, 5}
#define PIN_CHN_9 {DSY_GPIOB, 0}
#define PIN_CHN_10 {DSY_GPIOC, 0}
#define PIN_CHN_11 {DSY_GPIOC, 1}
#define PIN_CHN_14 {DSY_GPIOA, 2}
#define PIN_CHN_15 {DSY_GPIOA, 3}
#define PIN_CHN_16 {DSY_GPIOA, 0}
#define PIN_CHN_17 {DSY_GPIOA, 1}
#define PIN_CHN_18 {DSY_GPIOA, 4}
#define PIN_CHN_19 {DSY_GPIOA, 5}

static const uint32_t dsy_adc_channel_map[DSY_ADC_MAX_CHANNELS] = {
    ADC_CHANNEL_3,
    ADC_CHANNEL_4,
    ADC_CHANNEL_5,
    ADC_CHANNEL_7,
    ADC_CHANNEL_8,
    ADC_CHANNEL_9,
    ADC_CHANNEL_10,
    ADC_CHANNEL_11,
    ADC_CHANNEL_14,
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

//#define DSY_ADC_MAX_CHANNELS DSY_ADC_MAX_CHANNELS
#define DSY_ADC_MAX_MUX_CHANNELS 8
#define DSY_ADC_MAX_RESOLUTION 65536.0f

static uint16_t __attribute__((section(".sram1_bss")))
adc1_dma_buffer[DSY_ADC_MAX_CHANNELS];

static uint16_t __attribute__((section(".sram1_bss")))
adc1_mux_cache[DSY_ADC_MAX_CHANNELS][DSY_ADC_MAX_MUX_CHANNELS];

struct dsy_adc
{
    // channel data
    uint8_t   channels, mux_channels[DSY_ADC_MAX_CHANNELS];
    // Channel config ptr (has 'channels' elements)
    AdcChannelConfig *pin_cfg;
    // dma buffers
    uint16_t* dma_buffer;
    uint16_t (*mux_cache)[DSY_ADC_MAX_CHANNELS][DSY_ADC_MAX_MUX_CHANNELS];
    uint16_t mux_index[DSY_ADC_MAX_CHANNELS]; // 0->mux_channels per ADC channel
    //dsy_adc_handle* dsy_hadc;
};


ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
static dsy_adc    adc;

const uint32_t adc_channel_from_pin(dsy_gpio_pin *pin)
{
    // For now just a rough switch case for all ADC_CHANNEL values
    dsy_gpio_pin adcpins[DSY_ADC_MAX_CHANNELS] = 
    {
        {DSY_GPIOA, 6}, // CHN3
        {DSY_GPIOC, 4}, // CHN4
        {DSY_GPIOB, 1}, // CHN5
        {DSY_GPIOA, 7}, // CHN7
        {DSY_GPIOC, 5}, // CHN8
        {DSY_GPIOB, 0}, // CHN9
        {DSY_GPIOC, 0}, // CHN10
        {DSY_GPIOC, 1}, // CHN11
        {DSY_GPIOA, 2}, // CHN14
        {DSY_GPIOA, 3}, // CHN15
        {DSY_GPIOA, 0}, // CHN16
        {DSY_GPIOA, 1}, // CHN17
        {DSY_GPIOA, 4}, // CHN18
        {DSY_GPIOA, 5}, // CHN19
    };
    for (size_t i = 0; i < DSY_ADC_MAX_CHANNELS; i++)
    {
        if (dsy_pin_cmp(&adcpins[i], pin))
            return dsy_adc_channel_map[i];
    }
    return 0; // we should check what zero actually means in this context.
}



static void write_mux_value(uint8_t chn, uint8_t idx);


// BEGIN CPP VERSION
//
static void GenericAdcConfig(ADC_HandleTypeDef *hadc)
{
    hadc->Instance                      = ADC1;
    hadc->Init.ClockPrescaler           = ADC_CLOCK_ASYNC_DIV2;
    hadc->Init.Resolution               = ADC_RESOLUTION_16B;
    hadc->Init.ScanConvMode             = ADC_SCAN_ENABLE;
    hadc->Init.EOCSelection             = ADC_EOC_SEQ_CONV;
    hadc->Init.LowPowerAutoWait         = DISABLE;
    hadc->Init.ContinuousConvMode       = ENABLE;
    hadc->Init.NbrOfConversion          = adc.channels;
    hadc->Init.DiscontinuousConvMode    = DISABLE;
    hadc->Init.ExternalTrigConv         = ADC_SOFTWARE_START;
    hadc->Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc->Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    hadc->Init.Overrun                  = ADC_OVR_DATA_PRESERVED;
    hadc->Init.LeftBitShift             = ADC_LEFTBITSHIFT_NONE;
}

void AdcHandle::Init(AdcChannelConfig *cfg, size_t num_channels, OverSampling ovs)
{
    ADC_MultiModeTypeDef   multimode = {0};
    ADC_ChannelConfTypeDef sConfig   = {0};
    // Generic Init
    GenericAdcConfig(&hadc1);
    oversampling_ = ovs;
    // Set DMA buffers
    num_channels_ = num_channels;
    adc.dma_buffer = adc1_dma_buffer;
    adc.mux_cache  = &adc1_mux_cache;

    // Handle Oversampling
    for (size_t i = 0; i < num_channels_; i++)
    {
        config_[i] = cfg[i];
        adc.dma_buffer[i] = 0;
        adc.mux_channels[i] = cfg[i].mux_channels_;
    }
    if(oversampling_)
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
    switch(oversampling_)
    {
        case OVS_4:
            hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_2;
            hadc1.Init.Oversampling.Ratio         = 3;
            break;
        case OVS_8:
            hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_3;
            hadc1.Init.Oversampling.Ratio         = 7;
            break;
        case OVS_16:
            hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_4;
            hadc1.Init.Oversampling.Ratio         = 15;
            break;
        case OVS_32:
            hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_5;
            hadc1.Init.Oversampling.Ratio         = 31;
            break;
        case OVS_64:
            hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_6;
            hadc1.Init.Oversampling.Ratio         = 63;
            break;
        case OVS_128:
            hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_7;
            hadc1.Init.Oversampling.Ratio         = 127;
            break;
        case OVS_256:
            hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_8;
            hadc1.Init.Oversampling.Ratio         = 255;
            break;
        case OVS_512:
            hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_9;
            hadc1.Init.Oversampling.Ratio         = 511;
            break;
        case OVS_1024:
            hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_10;
            hadc1.Init.Oversampling.Ratio         = 1023;
            break;
        default: break;
    }
    // Init ADC
    if(HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        //Error_Handler();
    }
    // Configure the ADC multi-mode 
    multimode.Mode = ADC_MODE_INDEPENDENT;
    if(HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
    {
        //Error_Handler();
    }
    // Set Config Pointer and data for use in MspInit
    adc.pin_cfg = cfg;
    adc.channels = num_channels;
    // Configure Regular Channel 
    // Configure Shared settings for all channels.
    sConfig.SamplingTime = ADC_SAMPLETIME_8CYCLES_5;
    sConfig.SingleDiff   = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset       = 0;
    for(uint8_t i = 0; i < adc.channels; i++)
    {
        sConfig.Channel = adc_channel_from_pin(&config_[i].pin_.pin);
        sConfig.Rank    = dsy_adc_rank_map[i]; // This should still work.
        if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
        {
            // Error_Handler();
        }
    }
}



void AdcHandle::Start()
{
    HAL_ADCEx_Calibration_Start(
        &hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc.dma_buffer, adc.channels);
}

void AdcHandle::Stop()
{
    HAL_ADC_Stop_DMA(&hadc1);
}

uint16_t AdcHandle::Get(uint8_t chn)
{
    return adc.dma_buffer[chn < DSY_ADC_MAX_CHANNELS ? chn : 0];
}
uint16_t* AdcHandle::GetPtr(uint8_t chn)
{
    return &adc.dma_buffer[chn < DSY_ADC_MAX_CHANNELS ? chn : 0];
}

float AdcHandle::GetFloat(uint8_t chn)
{
    return (float)adc.dma_buffer[chn < DSY_ADC_MAX_CHANNELS ? chn : 0]
           / DSY_ADC_MAX_RESOLUTION;
}

uint16_t AdcHandle::GetMux(uint8_t chn, uint8_t idx)
{
    return *adc.mux_cache[chn < DSY_ADC_MAX_CHANNELS ? chn : 0][idx];
}

uint16_t* AdcHandle::GetMuxPtr(uint8_t chn, uint8_t idx)
{
    return adc.mux_cache[chn < DSY_ADC_MAX_CHANNELS ? chn : 0][idx];
}

float AdcHandle::GetMuxFloat(uint8_t chn, uint8_t idx)
{
    return (float)*adc.mux_cache[chn < DSY_ADC_MAX_CHANNELS ? chn : 0][idx]
           / DSY_ADC_MAX_RESOLUTION;
}

// END CPP VERSION


void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
    if(adcHandle->Instance == ADC1)
    {
        /* ADC1 clock enable */
        __HAL_RCC_ADC12_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        // GPIO Init has already happened. . . 
        for (size_t i = 0; i < adc.channels; i++)
        {
            dsy_gpio_init(&adc.pin_cfg[i].pin_);
        }
        /* ADC1 DMA Init */
        /* ADC1 Init */
        hdma_adc1.Instance                 = DMA1_Stream2;
        hdma_adc1.Init.Request             = DMA_REQUEST_ADC1;
        hdma_adc1.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        hdma_adc1.Init.PeriphInc           = DMA_PINC_DISABLE;
        hdma_adc1.Init.MemInc              = DMA_MINC_ENABLE;
        hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_adc1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
        hdma_adc1.Init.Mode                = DMA_CIRCULAR;
        hdma_adc1.Init.Priority            = DMA_PRIORITY_LOW;
        hdma_adc1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
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
        // We should probably hit the mux pins, too.
        for (size_t i = 0; i < adc.channels; i++)
        {
            dsy_gpio_deinit(&adc.pin_cfg[i].pin_);
        }
        HAL_DMA_DeInit(adcHandle->DMA_Handle);
    }
}

extern "C"
{
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
            uint8_t chn = i;
            uint8_t current_position = adc.mux_index[i];
            if(adc.mux_channels[chn] > 0)
            {
                // Capture current value to mux_cache
                *adc.mux_cache[i][current_position] = adc.dma_buffer[i];
                // Update Mux Position, and write GPIO
                adc.mux_index[chn] += 1;
                if(adc.mux_index[chn] > adc.mux_channels[chn])
                {
                    adc.mux_index[chn] = 0;
                }
                write_mux_value(chn, adc.mux_index[chn]);
            }
        }
    }
}
}

static void write_mux_value(uint8_t chn, uint8_t idx)
{
    dsy_gpio *p0, *p1, *p2;
    p0 = &adc.pin_cfg[chn].mux_pin_[0];
    p1 = &adc.pin_cfg[chn].mux_pin_[1];
    p2 = &adc.pin_cfg[chn].mux_pin_[2];
    dsy_gpio_write(p0, (idx & 0x01) > 0);
    dsy_gpio_write(p1, (idx & 0x02) > 0);
    dsy_gpio_write(p2, (idx & 0x04) > 0);
}
