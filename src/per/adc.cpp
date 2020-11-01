#include <stm32h7xx_hal.h>
#include "per/adc.h"
#include "util/hal_map.h"

using namespace daisy;

static void Error_Handler()
{
    asm("bkpt 255");
    while(1) {}
}

// Pinout by channel as dsy_gpio_pin
// TODO Figure out how to get this formatting
// to not suck..
#define PIN_CHN_3    \
    {                \
        DSY_GPIOA, 6 \
    }
#define PIN_CHN_4    \
    {                \
        DSY_GPIOC, 4 \
    }
#define PIN_CHN_5    \
    {                \
        DSY_GPIOB, 1 \
    }
#define PIN_CHN_7    \
    {                \
        DSY_GPIOA, 7 \
    }
#define PIN_CHN_8    \
    {                \
        DSY_GPIOC, 5 \
    }
#define PIN_CHN_9    \
    {                \
        DSY_GPIOB, 0 \
    }
#define PIN_CHN_10   \
    {                \
        DSY_GPIOC, 0 \
    }
#define PIN_CHN_11   \
    {                \
        DSY_GPIOC, 1 \
    }
#define PIN_CHN_14   \
    {                \
        DSY_GPIOA, 2 \
    }
#define PIN_CHN_15   \
    {                \
        DSY_GPIOA, 3 \
    }
#define PIN_CHN_16   \
    {                \
        DSY_GPIOA, 0 \
    }
#define PIN_CHN_17   \
    {                \
        DSY_GPIOA, 1 \
    }
#define PIN_CHN_18   \
    {                \
        DSY_GPIOA, 4 \
    }
#define PIN_CHN_19   \
    {                \
        DSY_GPIOA, 5 \
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
static const uint32_t adc_channel_from_pin(dsy_gpio_pin* pin);

static const uint32_t adc_channel_from_pin(dsy_gpio_pin* pin)
{
    // For now just a rough switch case for all ADC_CHANNEL values
    dsy_gpio_pin adcpins[DSY_ADC_MAX_CHANNELS] = {
        PIN_CHN_3,
        PIN_CHN_4,
        PIN_CHN_5,
        PIN_CHN_7,
        PIN_CHN_8,
        PIN_CHN_9,
        PIN_CHN_10,
        PIN_CHN_11,
        PIN_CHN_14,
        PIN_CHN_15,
        PIN_CHN_16,
        PIN_CHN_17,
        PIN_CHN_18,
        PIN_CHN_19,
    };
    for(size_t i = 0; i < DSY_ADC_MAX_CHANNELS; i++)
    {
        if(dsy_pin_cmp(&adcpins[i], pin))
            return dsy_adc_channel_map[i];
    }
    return 0; // we should check what zero actually means in this context.
}

// Declare Global ADC Handle
static dsy_adc adc;

// Begin AdcChannelConfig Implementations

void AdcChannelConfig::InitSingle(dsy_gpio_pin pin)
{
    pin_.pin      = pin;
    mux_channels_ = 0;
    pin_.mode     = DSY_GPIO_MODE_ANALOG;
    pin_.pull     = DSY_GPIO_NOPULL;
}
void AdcChannelConfig::InitMux(dsy_gpio_pin adc_pin,
                               size_t       mux_channels,
                               dsy_gpio_pin mux_0,
                               dsy_gpio_pin mux_1,
                               dsy_gpio_pin mux_2)
{
    size_t pins_to_init;
    // Init ADC Pin
    pin_.pin  = adc_pin;
    pin_.mode = DSY_GPIO_MODE_ANALOG;
    pin_.pull = DSY_GPIO_NOPULL;
    // Init Muxes
    mux_pin_[0].pin = mux_0;
    mux_pin_[1].pin = mux_1;
    mux_pin_[2].pin = mux_2;
    mux_channels_   = mux_channels < 8 ? mux_channels : 8;
    pins_to_init    = get_num_mux_pins_required(mux_channels_);
    for(size_t i = 0; i < pins_to_init; i++)
    {
        mux_pin_[i].mode = DSY_GPIO_MODE_OUTPUT_PP;
        mux_pin_[i].pull = DSY_GPIO_NOPULL;
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
    for(size_t i = 0; i < num_channels_; i++)
    {
        adc.pin_cfg[i]      = cfg[i];
        adc.dma_buffer[i]   = 0;
        adc.mux_channels[i] = cfg[i].mux_channels_;
    }
    adc.hadc1.Instance                      = ADC1;
    adc.hadc1.Init.ClockPrescaler           = ADC_CLOCK_ASYNC_DIV2;
    adc.hadc1.Init.Resolution               = ADC_RESOLUTION_16B;
    adc.hadc1.Init.ScanConvMode             = ADC_SCAN_ENABLE;
    adc.hadc1.Init.EOCSelection             = ADC_EOC_SEQ_CONV;
    adc.hadc1.Init.LowPowerAutoWait         = DISABLE;
    adc.hadc1.Init.ContinuousConvMode       = ENABLE;
    adc.hadc1.Init.NbrOfConversion          = adc.channels;
    adc.hadc1.Init.DiscontinuousConvMode    = DISABLE;
    adc.hadc1.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
    adc.hadc1.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc.hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    adc.hadc1.Init.Overrun                  = ADC_OVR_DATA_PRESERVED;
    adc.hadc1.Init.LeftBitShift             = ADC_LEFTBITSHIFT_NONE;
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
    adc.mux_used
        = false; // set false, and let any pin using it override this setting.
    for(uint8_t i = 0; i < adc.channels; i++)
    {
        const auto& cfg = adc.pin_cfg[i];

        // init ADC pin
        dsy_gpio_init(&cfg.pin_);

        // init mux pins (if any)
        adc.num_mux_pins_required[i]
            = get_num_mux_pins_required(cfg.mux_channels_);
        if(cfg.mux_channels_ > 0)
        {
            adc.mux_used = true;
            for(int j = 0; j < adc.num_mux_pins_required[i]; j++)
                dsy_gpio_init(&cfg.mux_pin_[j]);
        }

        // init adc channel sequence
        sConfig.Channel = adc_channel_from_pin(&adc.pin_cfg[i].pin_.pin);
        sConfig.Rank    = dsy_adc_rank_map[i];
        if(HAL_ADC_ConfigChannel(&adc.hadc1, &sConfig) != HAL_OK)
        {
            Error_Handler();
        }
    }
}

void AdcHandle::Start()
{
    size_t rx_size;
    HAL_ADCEx_Calibration_Start(
        &adc.hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);

    // When using the mux we use the HalfCplt callback to fill data, and the pins change
    // while converting the 'second half' of data (which just gets trashed for now).
    // This does affect the ADC read speed so we only do this when the mux is used.
    rx_size = adc.mux_used ? adc.channels * 2 : adc.channels;
    HAL_ADC_Start_DMA(&adc.hadc1, (uint32_t*)adc.dma_buffer, rx_size);
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
    dsy_gpio *p0, *p1, *p2;
    p0 = &adc.pin_cfg[chn].mux_pin_[0];
    dsy_gpio_write(p0, (idx & 0x01) > 0);
    if(num_mux_pins_to_write > 1)
    {
        p1 = &adc.pin_cfg[chn].mux_pin_[1];
        dsy_gpio_write(p1, (idx & 0x02) > 0);
    }
    if(num_mux_pins_to_write > 2)
    {
        p2 = &adc.pin_cfg[chn].mux_pin_[2];
        dsy_gpio_write(p2, (idx & 0x04) > 0);
    }
}

static void adc_internal_callback()
{
    // Handle Externally Multiplexed Pins
    // This is called from the HalfCpltCallback. The data from the second half will
    // be trash because the mux pins are being changed during conversion.
    //
    // We could set up everything to run from a timer and provide a few microseconds
    // to adjust pins before resuming reading, but that won't work with how everything
    // is set up right now.
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
}


// STM32 HAL function callbacks
void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
    if(adcHandle->Instance == ADC1)
    {
        // ADC1 clock enable
        __HAL_RCC_ADC12_CLK_ENABLE();

        // GPIO Init has already happened. . .
        // ADC1 DMA Init
        // ADC1 Init
        adc.hdma_adc1.Instance                 = DMA1_Stream2;
        adc.hdma_adc1.Init.Request             = DMA_REQUEST_ADC1;
        adc.hdma_adc1.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        adc.hdma_adc1.Init.PeriphInc           = DMA_PINC_DISABLE;
        adc.hdma_adc1.Init.MemInc              = DMA_MINC_ENABLE;
        adc.hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        adc.hdma_adc1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
        adc.hdma_adc1.Init.Mode                = DMA_CIRCULAR;
        adc.hdma_adc1.Init.Priority            = DMA_PRIORITY_LOW;
        adc.hdma_adc1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
        if(HAL_DMA_Init(&adc.hdma_adc1) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(adcHandle, DMA_Handle, adc.hdma_adc1);
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
            dsy_gpio_deinit(&adc.pin_cfg[i].pin_);
            for(size_t muxCh = 0; muxCh < adc.num_mux_pins_required[i]; muxCh++)
                dsy_gpio_deinit(&adc.pin_cfg[i].mux_pin_[muxCh]);
        }
        HAL_DMA_DeInit(adcHandle->DMA_Handle);
    }
}

extern "C"
{
    void DMA1_Stream2_IRQHandler(void) { HAL_DMA_IRQHandler(&adc.hdma_adc1); }

    void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
    {
        if(hadc->Instance == ADC1)
        {
            adc_internal_callback();
        }
    }
}
