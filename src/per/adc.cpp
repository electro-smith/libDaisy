#include <stm32h7xx_hal.h>
#include "per/adc.h"
#include "util/hal_map.h"

using namespace daisy;

static void Error_Handler()
{
    asm("bkpt 255");
    while(1) {}
}

#define DSY_ADC_MAX_MUX_CHANNELS 8
#define DSY_ADC_MAX_RESOLUTION 65536.0f

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

// Globals
// DMA Buffers
static uint16_t DMA_BUFFER_MEM_SECTION
    adc1_mux_cache[DSY_ADC_MAX_CHANNELS][DSY_ADC_MAX_MUX_CHANNELS];

/** Buffer for ADC Input channels
 ** It is 2x the number of channels for double-buffered support
 **
 ** Also used to provide buffer for trash data during mux pin changes.
 ***/
static uint16_t DMA_BUFFER_MEM_SECTION
    adc1_dma_buffer[DSY_ADC_MAX_CHANNELS * 2];

// Global ADC Struct
struct dsy_adc
{
    // Channel config ptr (has 'channels' elements)
    AdcChannelConfig pin_cfg[DSY_ADC_MAX_CHANNELS];
    uint8_t          num_mux_pins_required[DSY_ADC_MAX_CHANNELS];
    // channel data
    uint8_t  channels, mux_channels[DSY_ADC_MAX_CHANNELS];
    uint16_t mux_index[DSY_ADC_MAX_CHANNELS]; // 0->mux_channels per ADC channel
    // dma buffer ptrs
    uint16_t* dma_buffer;
    uint16_t (*mux_cache)[DSY_ADC_MAX_MUX_CHANNELS];
    ADC_HandleTypeDef hadc1;
    DMA_HandleTypeDef hdma_adc1;
    bool              mux_used; // flag set when mux is configured
};

// Static Functions
static int get_num_mux_pins_required(int num_mux_ch)
{
    if(num_mux_ch > 4)
        return 3;
    if(num_mux_ch > 2)
        return 2;
    if(num_mux_ch > 1)
        return 1;
    else
        return 0;
}
static void
                      write_mux_value(uint8_t chn, uint8_t idx, uint8_t num_mux_pins_to_write);
static const uint32_t adc_channel_from_pin(Pin* pin);

static const uint32_t adc_channel_from_pin(Pin* pin)
{
    // For now just a rough switch case for all ADC_CHANNEL values
    Pin adcpins[DSY_ADC_MAX_CHANNELS];

    adcpins[0].Init(Pin::Port::DSY_GPIOA, 6);
    adcpins[1].Init(Pin::Port::DSY_GPIOC, 4);
    adcpins[2].Init(Pin::Port::DSY_GPIOB, 1);
    adcpins[3].Init(Pin::Port::DSY_GPIOA, 7);
    adcpins[4].Init(Pin::Port::DSY_GPIOC, 5);
    adcpins[5].Init(Pin::Port::DSY_GPIOB, 0);
    adcpins[6].Init(Pin::Port::DSY_GPIOC, 0);
    adcpins[7].Init(Pin::Port::DSY_GPIOC, 1);
    adcpins[8].Init(Pin::Port::DSY_GPIOA, 2);
    adcpins[9].Init(Pin::Port::DSY_GPIOA, 3);
    adcpins[10].Init(Pin::Port::DSY_GPIOA, 0);
    adcpins[11].Init(Pin::Port::DSY_GPIOA, 1);
    adcpins[12].Init(Pin::Port::DSY_GPIOA, 4);
    adcpins[13].Init(Pin::Port::DSY_GPIOA, 5);

    for(size_t i = 0; i < DSY_ADC_MAX_CHANNELS; i++)
    {
        if(adcpins[i] == pin)
            return dsy_adc_channel_map[i];
    }
    return 0; // we should check what zero actually means in this context.
}

// Declare Global ADC Handle
static dsy_adc adc;

// Begin AdcChannelConfig Implementations

void AdcChannelConfig::InitSingle(Pin pin)
{
    pin_config_.pin  = pin;
    mux_channels_    = 0;
    pin_config_.mode = GPIO::Config::Mode::ANALOG;
}
void AdcChannelConfig::InitMux(Pin    adc_pin,
                               size_t mux_channels,
                               Pin    mux_0,
                               Pin    mux_1,
                               Pin    mux_2)
{
    size_t pins_to_init;
    // Init ADC Pin
    pin_config_.pin  = adc_pin;
    pin_config_.mode = GPIO::Config::Mode::ANALOG;
    // Init Muxes
    mux_pin_config_[0].pin = mux_0;
    mux_pin_config_[1].pin = mux_1;
    mux_pin_config_[2].pin = mux_2;
    mux_channels_          = mux_channels < 8 ? mux_channels : 8;
    pins_to_init           = get_num_mux_pins_required(mux_channels_);
    for(size_t i = 0; i < pins_to_init; i++)
    {
        mux_pin_config_[i].mode = GPIO::Config::Mode::OUTPUT_PP;
    }
}

// Begin AdcHandle Implementations

void AdcHandle::Init(AdcChannelConfig* cfg,
                     size_t            num_channels,
                     OverSampling      ovs)
{
    ADC_MultiModeTypeDef   multimode = {0};
    ADC_ChannelConfTypeDef sConfig   = {0};
    // Generic Init
    oversampling_ = ovs;
    // Set DMA buffers
    num_channels_  = num_channels;
    adc.dma_buffer = adc1_dma_buffer;
    adc.mux_cache  = &adc1_mux_cache[0];
    // Clear Buffers
    for(size_t i = 0; i < DSY_ADC_MAX_CHANNELS; i++)
    {
        adc.dma_buffer[i]   = 0;
        adc.mux_channels[i] = 0; // set to 0 mux first.
        adc.mux_index[i]    = 0;
    }
    // Set Config Pointer and data for use in MspInit
    adc.channels = num_channels;
    adc.mux_used
        = false; // set false, and let any pin using it override this setting.
    for(size_t i = 0; i < num_channels_; i++)
    {
        adc.pin_cfg[i]      = cfg[i];
        adc.dma_buffer[i]   = 0;
        adc.mux_channels[i] = cfg[i].mux_channels_;
        if(cfg[i].mux_channels_ > 0)
            adc.mux_used = true;
    }
    adc.hadc1.Instance                  = ADC1;
    adc.hadc1.Init.ClockPrescaler       = ADC_CLOCK_ASYNC_DIV2;
    adc.hadc1.Init.Resolution           = ADC_RESOLUTION_16B;
    adc.hadc1.Init.ScanConvMode         = ADC_SCAN_ENABLE;
    adc.hadc1.Init.EOCSelection         = ADC_EOC_SEQ_CONV;
    adc.hadc1.Init.LowPowerAutoWait     = DISABLE;
    adc.hadc1.Init.NbrOfConversion      = adc.channels;
    adc.hadc1.Init.ExternalTrigConv     = ADC_SOFTWARE_START;
    adc.hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;

    // Set ConversionDataManagement, and (Dis)Continuous based on whether
    // the callback needs to be used, or if the ADC can run in circular
    if(!adc.mux_used)
    {
        adc.hadc1.Init.ContinuousConvMode    = ENABLE;
        adc.hadc1.Init.DiscontinuousConvMode = DISABLE;
        adc.hadc1.Init.ConversionDataManagement
            = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    }
    else
    {
        adc.hadc1.Init.ContinuousConvMode    = DISABLE;
        adc.hadc1.Init.DiscontinuousConvMode = ENABLE;
        adc.hadc1.Init.ConversionDataManagement
            = ADC_CONVERSIONDATA_DMA_ONESHOT;
    }

    adc.hadc1.Init.Overrun      = ADC_OVR_DATA_PRESERVED;
    adc.hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
    // Handle Oversampling
    if(oversampling_)
    {
        adc.hadc1.Init.OversamplingMode = ENABLE;
        adc.hadc1.Init.Oversampling.OversamplingStopReset
            = ADC_REGOVERSAMPLING_CONTINUED_MODE;
        adc.hadc1.Init.Oversampling.TriggeredMode
            = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
        switch(oversampling_)
        {
            case OVS_4:
                adc.hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_2;
                adc.hadc1.Init.Oversampling.Ratio         = 3;
                break;
            case OVS_8:
                adc.hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_3;
                adc.hadc1.Init.Oversampling.Ratio         = 7;
                break;
            case OVS_16:
                adc.hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_4;
                adc.hadc1.Init.Oversampling.Ratio         = 15;
                break;
            case OVS_32:
                adc.hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_5;
                adc.hadc1.Init.Oversampling.Ratio         = 31;
                break;
            case OVS_64:
                adc.hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_6;
                adc.hadc1.Init.Oversampling.Ratio         = 63;
                break;
            case OVS_128:
                adc.hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_7;
                adc.hadc1.Init.Oversampling.Ratio         = 127;
                break;
            case OVS_256:
                adc.hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_8;
                adc.hadc1.Init.Oversampling.Ratio         = 255;
                break;
            case OVS_512:
                adc.hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_9;
                adc.hadc1.Init.Oversampling.Ratio         = 511;
                break;
            case OVS_1024:
                adc.hadc1.Init.Oversampling.RightBitShift
                    = ADC_RIGHTBITSHIFT_10;
                adc.hadc1.Init.Oversampling.Ratio = 1023;
                break;
            default: break;
        }
    }
    else
    {
        adc.hadc1.Init.OversamplingMode = DISABLE;
    }
    // Init ADC
    if(HAL_ADC_Init(&adc.hadc1) != HAL_OK)
    {
        Error_Handler();
    }
    // Configure the ADC multi-mode
    multimode.Mode = ADC_MODE_INDEPENDENT;
    if(HAL_ADCEx_MultiModeConfigChannel(&adc.hadc1, &multimode) != HAL_OK)
    {
        Error_Handler();
    }
    // Configure Regular Channel
    // Configure Shared settings for all channels.
    sConfig.SamplingTime = ADC_SAMPLETIME_8CYCLES_5;
    sConfig.SingleDiff   = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset       = 0;
    for(uint8_t i = 0; i < adc.channels; i++)
    {
        const auto& cfg = adc.pin_cfg[i];

        // init ADC pin
        cfg.pin_.Init(cfg.pin_config_);

        // init mux pins (if any)
        adc.num_mux_pins_required[i]
            = get_num_mux_pins_required(cfg.mux_channels_);
        if(cfg.mux_channels_ > 0)
        {
            for(int j = 0; j < adc.num_mux_pins_required[i]; j++)
                cfg.mux_pin_[j].Init(cfg.mux_pin_config_[j]);
        }

        // init adc channel sequence
        sConfig.Channel = adc_channel_from_pin(&adc.pin_cfg[i].pin_config_.pin);
        sConfig.Rank    = dsy_adc_rank_map[i];
        if(HAL_ADC_ConfigChannel(&adc.hadc1, &sConfig) != HAL_OK)
        {
            Error_Handler();
        }
    }
}

void AdcHandle::Start()
{
    HAL_ADCEx_Calibration_Start(
        &adc.hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&adc.hadc1, (uint32_t*)adc.dma_buffer, adc.channels);
}

void AdcHandle::Stop()
{
    HAL_ADC_Stop_DMA(&adc.hadc1);
}

// Accessors

uint16_t AdcHandle::Get(uint8_t chn) const
{
    return adc.dma_buffer[chn < DSY_ADC_MAX_CHANNELS ? chn : 0];
}
uint16_t* AdcHandle::GetPtr(uint8_t chn) const
{
    return &adc.dma_buffer[chn < DSY_ADC_MAX_CHANNELS ? chn : 0];
}

float AdcHandle::GetFloat(uint8_t chn) const
{
    return (float)adc.dma_buffer[chn < DSY_ADC_MAX_CHANNELS ? chn : 0]
           / DSY_ADC_MAX_RESOLUTION;
}

uint16_t AdcHandle::GetMux(uint8_t chn, uint8_t idx) const
{
    return adc.mux_cache[chn < DSY_ADC_MAX_CHANNELS ? chn : 0][idx];
}

uint16_t* AdcHandle::GetMuxPtr(uint8_t chn, uint8_t idx) const
{
    return &adc.mux_cache[chn < DSY_ADC_MAX_CHANNELS ? chn : 0][idx];
}

float AdcHandle::GetMuxFloat(uint8_t chn, uint8_t idx) const
{
    return (float)adc.mux_cache[chn < DSY_ADC_MAX_CHANNELS ? chn : 0][idx]
           / DSY_ADC_MAX_RESOLUTION;
}


// Internal Implementations

static void
write_mux_value(uint8_t chn, uint8_t idx, uint8_t num_mux_pins_to_write)
{
    adc.pin_cfg[chn].mux_pin_[0].Write((idx & 0x01) > 0);
    if(num_mux_pins_to_write > 1)
    {
        adc.pin_cfg[chn].mux_pin_[1].Write((idx & 0x02) > 0);
    }
    if(num_mux_pins_to_write > 2)
    {
        p2 = &adc.pin_cfg[chn].mux_pin_[2].Write((idx & 0x04) > 0);
    }
}

static void adc_init_dma1()
{
    adc.hdma_adc1.Instance                 = DMA1_Stream2;
    adc.hdma_adc1.Init.Request             = DMA_REQUEST_ADC1;
    adc.hdma_adc1.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    adc.hdma_adc1.Init.PeriphInc           = DMA_PINC_DISABLE;
    adc.hdma_adc1.Init.MemInc              = DMA_MINC_ENABLE;
    adc.hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    adc.hdma_adc1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    if(adc.mux_used)
        adc.hdma_adc1.Init.Mode = DMA_NORMAL;
    else
        adc.hdma_adc1.Init.Mode = DMA_CIRCULAR;
    adc.hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    adc.hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if(HAL_DMA_Init(&adc.hdma_adc1) != HAL_OK)
    {
        Error_Handler();
    }

    //__HAL_LINKDMA(&adc.hadc1, DMA_Handle, adc.hdma_adc1);
}

// Handle Externally Multiplexed Pins
// This is called from the CpltCallback, only when at least one multiplexor is used.
// When this is the case the DMA is also configured in normal mode instead of circular.
//
// This allows the DMA to stop while the GPIO switch, and then once that is done
// the DMA Transfer is started again. This prevents issues with data being read to the wrong channels
//
// We could set up everything to run from a timer and provide a few microseconds
// to adjust pins before resuming reading, but that won't work with how everything
// is set up right now.
static void adc_internal_callback()
{
    for(uint16_t i = 0; i < adc.channels; i++)
    {
        const uint8_t chn              = i;
        const uint8_t current_position = adc.mux_index[i];
        if(adc.mux_channels[chn] > 0)
        {
            // Capture current value to mux_cache
            const auto value                   = adc.dma_buffer[i];
            adc.mux_cache[i][current_position] = value;
            // Update Mux Position, and write GPIO
            adc.mux_index[chn] += 1;
            if(adc.mux_index[chn] >= adc.mux_channels[chn])
                adc.mux_index[chn] = 0;
            write_mux_value(
                chn, adc.mux_index[chn], adc.num_mux_pins_required[chn]);
        }
    }
    // Restart DMA
    adc_init_dma1();
    HAL_ADC_Start_DMA(&adc.hadc1, (uint32_t*)adc.dma_buffer, adc.channels);
}


// STM32 HAL function callbacks
void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
    if(adcHandle->Instance == ADC1)
    {
        // ADC1 clock enable
        __HAL_RCC_ADC12_CLK_ENABLE();
        adc_init_dma1();
        __HAL_LINKDMA(&adc.hadc1, DMA_Handle, adc.hdma_adc1);
    }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{
    if(adcHandle->Instance == ADC1)
    {
        // Peripheral clock disable
        __HAL_RCC_ADC12_CLK_DISABLE();
        // deinit pins
        for(size_t i = 0; i < adc.channels; i++)
        {
            adc.pin_cfg[i].pin_.DeInit();
            for(size_t muxCh = 0; muxCh < adc.num_mux_pins_required[i]; muxCh++)
                adc.pin_cfg[i].mux_pin_[muxCh].DeInit();
        }
        HAL_DMA_DeInit(adcHandle->DMA_Handle);
    }
}

extern "C"
{
    void DMA1_Stream2_IRQHandler(void) { HAL_DMA_IRQHandler(&adc.hdma_adc1); }

    void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
    {
        if(hadc->Instance == ADC1 && adc.mux_used)
        {
            adc_internal_callback();
        }
    }

    /*
     * For some reason the flags for injected conversions were getting set.
     * When I went to implement a simple callback to clear them it stopped happening
     * So I guess this doesn't need to be here..
     */
    void HAL_Injected_ConvCpltCallback(ADC_HandleTypeDef* hadc)
    {
#if DEBUG
        asm("bkpt 255");
#endif
    }

    void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc)
    {
        if(hadc->Instance == ADC1)
        {
#if DEBUG
            asm("bkpt 255");
#endif
        }
    }
}
